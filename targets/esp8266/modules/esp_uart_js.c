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

static void serial_flush()
{
  while(uart_rxfifo_wait (UART_NUM, 0))
  {
    uart_getc (UART_NUM);
  }
}

DELCARE_HANDLER (uart_flush)
{
  if (args_cnt != 0)
  {
    return raise_argument_count_error (UART_OBJECT_NAME, UART_FLUSH, "0");
  }

  serial_flush();

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

/* GLOBAL STATIC DECLARATIONS */
static uint8_t generic_ack_reply[] = {0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00};
static uint8_t sync_ack_reply_ext[] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
static int command_length = 6;
static int image_pos = 0;
static int imageSize = 0;
static int sync_attempts_max = 5;
static bool wifi_been_connected = false;
static const int len_commands = 6;

/* UTILITY FUNCTIONS */

static void delay_millies(int ms)
{
  vTaskDelay (ms / portTICK_PERIOD_MS);
}

static void reset_device(int port)
{
  gpio_write (port, 0);
  delay_millies(900);
  gpio_write (port, 1);
}

static void serial_write_buffer(uint8_t* buff, int len)
{
  for (size_t i = 0; i < len; i++) {
    uart_putc (UART_NUM, buff[i]);
  }
}

static bool serial_available() // Always called without arguments
{
  return uart_rxfifo_wait (UART_NUM, 0);
}

static int serial_read()
{
  return uart_getc (UART_NUM);
}

static uint8_t _commandID (uint8_t* cmd)
{
  return cmd[1];
}

static bool _waitForBytes(uint8_t* sentCommand, uint8_t* ackReply)
{
  uint8_t reply[6];

  memcpy (reply, ackReply, sizeof (reply));

  if (ackReply != sync_ack_reply_ext) {
    reply[2] = _commandID(sentCommand);
  }

  int found_bytes = 0;
  int i = 0;

  while (serial_available() && i < command_length) {
    if (serial_read() == reply[i] || reply[i] == 0) {
      found_bytes++;
    }
    i++;
  }

  return found_bytes == command_length;
}

static bool attempt_sync ()
{
  int attempts = 0;
  uint8_t sync_command[] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
  uint8_t sync_final_command[] = {0xAA, 0x0E, 0x0D, 0x00, 0x00, 0x00};

  while (attempts < 60)
  {
    serial_flush();
    serial_write_buffer(sync_command, len_commands);

    delay_millies(50 + attempts);

    if (_waitForBytes (sync_command, generic_ack_reply))
    {
      printf("1st if\n");
      if (_waitForBytes (sync_command, sync_ack_reply_ext))
      {
        printf("2nd if\n");
        delay_millies(50);
        serial_write_buffer (sync_final_command, len_commands);
        return true;
      }
    }
    attempts++;
  }

  return false;
}

static bool _sendCommand (uint8_t* cmd)
{
  delay_millies(100);
  serial_write_buffer(cmd, len_commands);
  delay_millies(500);

  return _waitForBytes(cmd, generic_ack_reply);
}

static bool _sendInitial ()
{
  uint8_t initial_command[] = {0xAA, 0x01, 0x00, 0x07, 0x07, 0x07 };

  serial_flush();

  return _sendCommand(initial_command);
}

static bool _setPackageSize()
{
  uint8_t pack_size[] = {0xAA, 0x06, 0x08, 0x80, 0x00, 0x00 };
  return _sendCommand(pack_size);
}

static bool _doSnapshot()
{
  uint8_t snapshot[] = {0xAA, 0x05, 0x00, 0x00, 0x00, 0x00 };
  return _sendCommand(snapshot);
}

static bool _getPicture()
{
  uint8_t get_picture[] = {0xAA, 0x04, 0x01, 0x00, 0x00, 0x00};
  int ack[6];
  if (_sendCommand(get_picture)) {
    for (int i = 0; i < 6; i++) {
      while (!serial_available());
      ack[i] = serial_read();
    }

    /*?*/
    imageSize = (imageSize << 8) | ack[5];
    imageSize = (imageSize << 8) | ack[4];
    imageSize = (imageSize << 8) | ack[3];

    image_pos = imageSize;

    if (imageSize > 0){
      return true;
    }
  }
  return false;
}

static bool wifi_available()
{
  return sdk_wifi_station_get_connect_status () == STATION_GOT_IP;
}

static void wifi_connect(char* ssid, size_t ssid_req_sz, char* password, size_t password_req_sz)
{
  if (wifi_been_connected)
  {
    printf("Wifi is already connected!");
  }

  struct sdk_station_config config;
  uint32_t buffer_size = ssid_req_sz > password_req_sz ? ssid_req_sz + 1 : password_req_sz + 1;
  char* str_buf_p;
  str_buf_p = (char* ) malloc (sizeof (char) * buffer_size);

  memcpy (str_buf_p, ssid, ssid_req_sz);
  printf("%s\n", str_buf_p);
  str_buf_p[ssid_req_sz] = 0;
  printf("%s\n", str_buf_p);
  strcpy((char *) config.ssid, (char *) str_buf_p);
  memcpy (str_buf_p, password, password_req_sz);
  printf("%s\n", str_buf_p);
  str_buf_p[password_req_sz] = 0;
  printf("%s\n", str_buf_p);
  strcpy((char *) config.password, (char *) str_buf_p);
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
}

static bool wifi_send(char* ip, size_t ip_size , int port_no, uint8_t* imageBuffer, char* picture_name, size_t picture_name_size, size_t image_bytes)
{
  /* Is it acceptable this way? for original method, see esp_wifi_js.c: wifi_send */
  char* str_buf_p = (char* ) malloc (ip_size);
  memcpy (str_buf_p, ip, ip_size);

  uint32_t port = (uint32_t) port_no;

  char* file_name_buf_p = (char* ) malloc (picture_name_size);
  memcpy (file_name_buf_p, picture_name, picture_name_size);

  uint32_t data_length = (uint32_t) image_bytes;
  jerry_value_t source = jerry_create_undefined(); /**???*/

  // Not needed?
  // void *native_p;
  // const jerry_object_native_info_t *type_p;
  // bool has_p = jerry_get_object_native_pointer (args_p[2], &native_p, &type_p);
  // if (has_p && type_p == get_native_file_obj_type_info())
  // {
  //   FIL *f = native_p;
  //   send_data_on_tcp (0, data_length, (const char *) str_buf_p, port, file_name_buf_p, picture_name_size, f);
  //   return true;
  // }
  bool success;
  source = jerry_value_to_string ((int) imageBuffer);
  if (send_data_on_tcp (source, data_length, (const char *) str_buf_p, port, (jerry_char_t*) file_name_buf_p, picture_name_size, NULL))
  {
    printf("Succesful sending\n");
    success = true;
  }
  else
  {
    printf("Sending failed\n");
    success = false;
  }
  free(str_buf_p);
  free(file_name_buf_p);
  return success;
}

/* END OF UTILITY FUNCTIONS */

/* CORE FUNCTIONS */

static bool init (int reset_pin)
{
  int attempts = 0;

  reset_device(reset_pin);

  while (attempts < sync_attempts_max)
  {
    printf ("Attempt %d\n", attempts);

    if (attempt_sync ())
    {
      printf ("CONNECTED\n");
      delay_millies (2000);
      return true;
    }

    reset_device(reset_pin);
    attempts++;
  }

  return false;
}

static bool take_picture ()
{
  if(_sendInitial())
  {
    printf("_sendInitial succesful\n");
    if(_setPackageSize())
    {
      printf("_setPackageSize succesful\n");
      if(_doSnapshot())
      {
        printf("_doSnapshot succesful\n");
        if(_getPicture())
        {
          printf ("The picture has been taken!\n");
          return true;
        }
      }
    }
  }

  return false;
}

static bool storePicture (char* image_name, uint8_t* imageBuffer, size_t imageBuffer_size)
{
  uint8_t ack[] = {0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00};
  int bytes;

  if (image_pos == 0) {
    return false;
  }

  // var image = SD.open(path, SD.appendWrite);
  int counter = 0;
  while (image_pos > 0) {
    if (image_pos < imageBuffer_size) {
      bytes = image_pos + command_length;
    }
    else {
      bytes = imageBuffer_size + command_length;
    }
    ack[4] = counter++;
    serial_write_buffer(ack, len_commands);
    delay_millies(45);
    int attempts = 0;
    while (serial_available() != bytes && attempts++ < sync_attempts_max) {
      delay_millies(30);
    }

    if (attempts == sync_attempts_max){
      printf ("Failed to store data!");
      return false;
    }

    int image_bytes = 0;
    if (!wifi_available()) {
      wifi_connect("ESP8266", sizeof("ESP8266"), "Barackospite", sizeof("Barackospite"));
    }
    for (int i = 0; i < bytes; i++) {
      int s = serial_read();
      if (i >= 4 && i < bytes - 2) {
        imageBuffer[i - 4] = s;
        image_pos--;
        image_bytes++;
        if (wifi_send("10.109.165.100", sizeof("10.109.165.100"), 5002, imageBuffer, "/picc.jpg", sizeof("/picc.jpg"), image_bytes))
        {
          printf ("Data sent\n");
        } else {
          printf ("Data can not be sent\n");
          return false;
        }
      }
    }
    // SD.write(image, this.imageBuffer, SD.asBinary, image_bytes); // WIFI send innen
  }
  // SD.close (image);

  ack[4] = 0xF0;
  ack[5] = 0xF0;
  printf("o\n");
  serial_write_buffer(ack, len_commands);
  printf ("The picture has been stored!");
  return true;
}
/* place for store function */

/* END OF CORE FUNCTIONS */

DELCARE_HANDLER (uart_init_take_store){
  uint8_t imageBuffer[122];
  /* Initialize camera */
  /** uart_init */
  size_t imageBuffer_size = sizeof(imageBuffer);
  char* image_name = "pic.jpg";
  int reset_pin = -1;
  int baud_rate = -1;

  if (args_cnt > 0 && jerry_value_is_number (args_p[0]))
  {
    reset_pin = (int) jerry_get_number_value (args_p[0]);
  }
  else
  {
    printf("Missing reset pin number\n");
    return jerry_create_boolean (false);
  }

  if (args_cnt > 1 && jerry_value_is_number (args_p[1]))
  {
    baud_rate = (int) jerry_get_number_value (args_p[1]);
  }
  else
  {
    printf("Missing baud rate\n");
    return jerry_create_boolean (false);
  }

  uart_set_baud(UART_NUM, baud_rate);

  /* Reset the camera */
  gpio_enable (reset_pin, 1);

  /* Init */
  if(init (reset_pin))
  {
    /* Take picture*/
    if(take_picture ())
    {
      storePicture(image_name, imageBuffer, imageBuffer_size);
    } else {
      printf ("Cannot take picture with uCam-iii!");
    }
  } else {
    printf ("Cannot sync with uCam-iii!");
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
  register_native_function (UART_INIT_TAKE_STORE, uart_init_take_store_handler, uart_object);

  jerry_release_value (uart_object);
}
