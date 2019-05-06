#include "esp_uart_js.h"
#include "esp_wifi_js.h"

DELCARE_HANDLER (uart_init)
{
  if (args_cnt != 1)
  {
    return raise_argument_count_error (UART_OBJECT_NAME, UART_INIT, "1");
  }

  if (!jerry_value_is_number (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_NUMBER);
  }

  uint32_t baud = jerry_get_number_value (args_p[0]);

  uart_set_baud(UART_NUM, baud);
  return jerry_create_boolean (true);
}

DELCARE_HANDLER (uart_available)
{
  if (args_cnt != 0 && args_cnt != 1)
  {
    return raise_argument_count_error (UART_OBJECT_NAME, UART_AVAILABLE, "0 or 1");
  }

  if (args_cnt == 1 && !jerry_value_is_number (args_p[0]))
  {
    return raise_argument_type_error ("1", TYPE_NUMBER);
  }

  if (args_cnt == 1)
  {
    int32_t bytes = jerry_get_number_value (args_p[0]);
    return jerry_create_number (uart_rxfifo_wait (UART_NUM, bytes));
  }

  return jerry_create_number (uart_rxfifo_wait (UART_NUM, 0));
}

DELCARE_HANDLER (uart_read)
{
  if (args_cnt != 0)
  {
    return raise_argument_count_error (UART_OBJECT_NAME, UART_READ, "0");
  }

  return jerry_create_number (uart_getc (UART_NUM));
}

DELCARE_HANDLER (uart_flush)
{
  if (args_cnt != 0)
  {
    return raise_argument_count_error (UART_OBJECT_NAME, UART_FLUSH, "0");
  }

  while(uart_rxfifo_wait (UART_NUM, 0))
  {
    uart_getc (UART_NUM);
  }

  return jerry_create_boolean (true);
}

DELCARE_HANDLER (uart_write){
  if (args_cnt != 1)
  {
    return raise_argument_count_error (UART_OBJECT_NAME, UART_WRITE, "0");
  }

  if (jerry_value_is_number (args_p[0]))
  {
    int value = (int) jerry_get_number_value (args_p[0]);
    uart_putc (UART_NUM, value);
  }
  else if (jerry_value_is_array(args_p[0]))
  {
    uint32_t len = jerry_get_array_length (args_p[0]);
    uint8_t element = 0;

    for (int i = 0; i < len; i++)
    {
      jerry_value_t idx = jerry_get_property_by_index (args_p[0], i);
      element = (int) jerry_get_number_value (idx);
      uart_putc (UART_NUM, element);
      jerry_release_value (idx);
    }
  }
  else
  {
    return raise_argument_type_error ("1", TYPE_NUMBER "or" TYPE_ARRAY);
  }

  return jerry_create_boolean (true);
}

void
register_uart_object (jerry_value_t global_object)
{
  jerry_value_t uart_object = jerry_create_object ();
  register_js_value_to_object (UART_OBJECT_NAME, uart_object, global_object);

  register_native_function (UART_INIT, uart_init_handler, uart_object);
  register_native_function (UART_READ, uart_read_handler, uart_object);
  register_native_function (UART_WRITE, uart_write_handler, uart_object);
  register_native_function (UART_AVAILABLE, uart_available_handler, uart_object);
  register_native_function (UART_FLUSH, uart_flush_handler, uart_object);

  jerry_release_value (uart_object);
}

DELCARE_HANDLER (uart_init_take_store){
  buffer = int[122];
  imageBuffer = uint8_t[buffer];
  int command_length = 6;
  sync_attempts_max = 5;
  /* Initialize camera */
  /** uart_init */
  int reset_pin = 5;
  uint32_t baud = jerry_get_number_value (reset_pin);

  uart_set_baud(UART_NUM, baud);

  /* Take picture */

  /** Send initial */
  /*** Serial.flush */
  while(uart_rxfifo_wait (UART_NUM, 0))
  {
    uart_getc (UART_NUM);
  }
  /*** Serial.write(cmd) (array) */
  uint8_t initial_command = [0xAA, 0x01, 0x00, 0x07, 0x07, 0x07 ];

  uint32_t len = jerry_get_array_length (initial_command);
  uint8_t element = 0;

  for (int i = 0; i < len; i++)
  {
    jerry_value_t idx = jerry_get_property_by_index (initial_command, i);
    element = (int) jerry_get_number_value (idx);
    uart_putc (UART_NUM, element);
    jerry_release_value (idx);
  }

  /** Set package size */
  /*** Serial.write(cmd) (array) */
  uint8_t pack_size[] = [0xAA, 0x06, 0x08, 0x80, 0x00, 0x00 ];

  len = jerry_get_array_length (pack_size);
  element = 0;

  for (int i = 0; i < len; i++)
  {
    jerry_value_t idx = jerry_get_property_by_index (pack_size, i);
    element = (int) jerry_get_number_value (idx);
    uart_putc (UART_NUM, element);
    jerry_release_value (idx);
  }

  /** Do snapshot */
  /*** Serial.write(cmd) (array) */
  uint8_t snapshot = [0xAA, 0x05, 0x00, 0x00, 0x00, 0x00 ];

  len = jerry_get_array_length (snapshot);
  element = 0;

  for (int i = 0; i < len; i++)
  {
    jerry_value_t idx = jerry_get_property_by_index (snapshot, i);
    element = (int) jerry_get_number_value (idx);
    uart_putc (UART_NUM, element);
    jerry_release_value (idx);
  }

  /** Get picture */
  int ack[6];
  /*** Serial.write(cmd) (array) */
  uint8_t get_picture = [0xAA, 0x04, 0x01, 0x00, 0x00, 0x00];

  len = jerry_get_array_length (get_picture);
  element = 0;

  for (int i = 0; i < len; i++)
  {
    jerry_value_t idx = jerry_get_property_by_index (get_picture, i);
    element = (int) jerry_get_number_value (idx);
    uart_putc (UART_NUM, element);
    jerry_release_value (idx);
  }


  for (int i = 0; i < 6; i++) {
    /*** Serial.avaliable */
    while (!uart_rxfifo_wait (UART_NUM, 0));
    /*** Serial.read */
    ack[i] = uart_getc (UART_NUM)
  }

  int imageSize = 0;
  imageSize = (imageSize << 8) | ack[5];
  imageSize = (imageSize << 8) | ack[4];
  imageSize = (imageSize << 8) | ack[3];
  int image_pos = imageSize;

  /* Store picture */
  int ack = [0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00];
  int bytes;

  int counter = 0;
  while (image_pos > 0) {
    if (image_pos < 122) {
      bytes = image_pos + command_length;
    }
    else {
      bytes = 122 + command_length;
    }
    ack[4] = counter++;
    /** Serial.write (array) */
    len = jerry_get_array_length (ack);
    element = 0;

    for (int i = 0; i < len; i++)
    {
      jerry_value_t idx = jerry_get_property_by_index (ack, i);
      element = (int) jerry_get_number_value (idx);
      uart_putc (UART_NUM, element);
      jerry_release_value (idx);
    }

    /**Delay millis TODO */

    int attempts = 0;
    while (uart_rxfifo_wait (UART_NUM, 0) != bytes && attempts++ < sync_attempts_max) {
    /*** DELAY.millis(30); TODO */
    }

    if (attempts == sync_attempts_max) {
      printf ("Failed to store data!");
    }

    int image_bytes = 0;

    if (sdk_wifi_station_get_connect_status () == STATION_GOT_IP) {
      /*** WIFI.connect */
      char* ssid = "ESP8266";
      char* password = "Barackospite";
      struct sdk_station_config config;
      jerry_size_t ssid_req_sz = jerry_get_string_length (ssid);
      jerry_size_t password_req_sz = jerry_get_string_length (password);
      uint32_t buffer_size = ssid_req_sz > password_req_sz ? ssid_req_sz + 1 : password_req_sz + 1;
      jerry_char_t *str_buf_p;
      str_buf_p = (jerry_char_t *) malloc (sizeof (jerry_char_t) * buffer_size);

      jerry_string_to_char_buffer (ssid, str_buf_p, ssid_req_sz);
      str_buf_p[ssid_req_sz] = 0;
      strcpy((char *) ssid, (char *) str_buf_p);
      jerry_string_to_char_buffer (password, str_buf_p, password_req_sz);
      str_buf_p[password_req_sz] = 0;
      strcpy((char *) password, (char *) str_buf_p);
      free (str_buf_p);
      sdk_wifi_set_opmode (STATION_MODE);
      sdk_wifi_station_set_config (&config);
      sdk_wifi_station_connect ();
      sdk_wifi_station_set_auto_connect (0);

      uint8_t attempts = 0;
      while (sdk_wifi_station_get_connect_status () != STATION_GOT_IP && attempts < WIFI_MAX_CONNECT_ATTEMPTS)
      {
        vTaskDelay (100 / portTICK_PERIOD_MS);
        taskYIELD();
        attempts++;
      }
      if (attempts == WIFI_MAX_CONNECT_ATTEMPTS)
      {
        printf("Cannot connect to the given AP!");
      }

      if (!sntp_been_init())
      {
        init_esp_sntp ();
      }

      wifi_been_connected = true;

      /** End of WIFI.connect */
    }

    for (int i = 0; i < bytes; i++) {
      int s = uart_getc (UART_NUM);
      if (i >= 4 && i < bytes - 2) {
        imageBuffer[i - 4] = s;
        image_pos--;
        image_bytes++;
        /** WIFI.send */
        char* ip = "10.109.165.100";
        int port = 5002;
        char* filename = "/picc.jpg"

        jerry_size_t req_sz = jerry_get_string_length (ip);
        jerry_char_t str_buf_p[req_sz + 1];
        jerry_string_to_char_buffer (ip, str_buf_p, req_sz);
        str_buf_p[req_sz] = 0;

        uint32_t port = jerry_get_number_value (port);

        jerry_size_t file_name_req_sz = jerry_get_string_length (filename);
        jerry_char_t file_name_buf_p[file_name_req_sz + 1];
        jerry_string_to_char_buffer (filename, file_name_buf_p, file_name_req_sz);
        file_name_buf_p[file_name_req_sz] = 0;

        uint32_t data_length = jerry_get_number_value (image_bytes);
        jerry_value_t source = jerry_create_undefined();

        if (jerry_value_is_object (imageBuffer))
        {
          void *native_p;
          const jerry_object_native_info_t *type_p;
          bool has_p = jerry_get_object_native_pointer (imageBuffer, &native_p, &type_p);
          if (has_p && type_p == get_native_file_obj_type_info())
          {
            FIL *f = native_p;
            send_data_on_tcp (0, data_length, (const char *) str_buf_p, port, file_name_buf_p, file_name_req_sz, f);
            return jerry_create_boolean (true);
          }
        }

        source = jerry_value_to_string (imageBuffer);
        return send_data_on_tcp (source, data_length, (const char *) str_buf_p, port, file_name_buf_p, file_name_req_sz, NULL);
        /** End of WIFI.send */

        ack[4] = 0xF0;
        ack[5] = 0xF0;
        /** Serial.write */
        len = jerry_get_array_length (ack);
        element = 0;

        for (int i = 0; i < len; i++)
        {
          jerry_value_t idx = jerry_get_property_by_index (ack, i);
          element = (int) jerry_get_number_value (idx);
          uart_putc (UART_NUM, element);
          jerry_release_value (idx);
        }

        printf ("The picture has been stored!");

        // WIFI.send("10.109.165.100", 5002, imageBuffer, "/picc.jpg", image_bytes)
      }
    }
  }


}
