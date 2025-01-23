/* WebSocket Echo Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#include <device_config.h>
/* A simple example that demonstrates using websocket echo server
 */
static const char *TAG = "ws_echo_server";

/*
 * Structure holding server handle
 * and internal socket fd in order
 * to use out of request send
 */

// HTML 页面内容
const char root_html[] = 
    "<!DOCTYPE html>"
    "<html lang=\"en\">"
    "<head>"
    "<meta charset=\"UTF-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<title>ESP32 Web Server</title>"
    "<style>"
    "body { font-family: Arial, sans-serif; background-color: #f4f4f4; padding: 20px; }"
    "h1 { color: #333; }"
    "p { font-size: 16px; color: #555; }"
    "a { color: #0066cc; text-decoration: none; }"
    "a:hover { text-decoration: underline; }"
    "</style>"
    "</head>"
    "<body>"
    "<h1>Welcome to the ESP32 Web Server!</h1>"
    "<p>This is the root page served by the ESP32 web server.</p>"
    "<p><a href=\"/update_html\">Go to another page</a></p>"
    "</body>"
    "</html>";

// root_start 和 root_end 是静态内容的起始和结束位置

static esp_err_t root_get_handler(httpd_req_t *req)
{

    ESP_LOGI(TAG, "Serve root");
    // 设置返回类型为 text/html
    httpd_resp_set_type(req, "text/html");
    // 发送静态 HTML 页面
    httpd_resp_send(req, (const char*)root_html, strlen(root_html));

    return ESP_OK;
}

static const httpd_uri_t root = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_get_handler,
    .is_websocket = true
};

const char update_html[] =
    "<!DOCTYPE html>"
    "<html>"
        "<head>"
            "<title>ESP32 OTA Firmware Upload</title>"
        "</head>"
        "<body>"
            "<h1>ESP32 OTA Firmware Upload</h1>"
            "<form method="POST" action="/update" enctype="multipart/form-data">"
            "<label for="firmware">Select firmware file (.bin):</label>"
            "<input type="file" name="firmware" accept=".bin" required><br><br>"
            "<input type="submit" value="Upload Firmware">"
            "</form>"
        "</body>"
    "</html>";


static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Registering the ws handler
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &root);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static esp_err_t stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}


// static void disconnect_handler(void* arg, esp_event_base_t event_base,
//                                int32_t event_id, void* event_data)
// {
//     httpd_handle_t* server = (httpd_handle_t*) arg;
//     if (*server) {
//         ESP_LOGI(TAG, "Stopping webserver");
//         if (stop_webserver(*server) == ESP_OK) {
//             *server = NULL;
//         } else {
//             ESP_LOGE(TAG, "Failed to stop http server");
//         }
//     }
// }

// static void connect_handler(void* arg, esp_event_base_t event_base,
//                             int32_t event_id, void* event_data)
// {
//     httpd_handle_t* server = (httpd_handle_t*) arg;
//     if (*server == NULL) {
//         ESP_LOGI(TAG, "Starting webserver");
//         *server = start_webserver();
//     }
// }



void wifi_ap_test(void)
{
    esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    // esp_netif_ip_info_t ip_info;
    // IP4_ADDR(&ip_info.ip, 192,168,10,1);      // 设置静态 IP 地址
    // IP4_ADDR(&ip_info.gw, 192,168,10,1);      // 设置网关（AP 的 IP 地址）
    // IP4_ADDR(&ip_info.netmask, 255,255,255,0); // 设置子网掩码

    // // 设置 IP 地址、网关和子网掩码
    // esp_netif_set_ip_info(ap_netif, &ip_info);

    wifi_init_config_t wifi_cfg=WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len= strlen(EXAMPLE_ESP_WIFI_SSID),
            .password = EXAMPLE_ESP_WIFI_PASS,//此处的密码长度不能小于8位
            .max_connection = 2,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));

    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config) );
    
    ESP_ERROR_CHECK(esp_wifi_start());

    // esp_netif_ip_info_t ip_info;
    // esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info);

    // char ip_addr[16];
    // inet_ntoa_r(ip_info.ip.addr, ip_addr, 16);
    // ESP_LOGI(TAG, "Set up softAP with IP: %s", ip_addr);

    // ESP_LOGI(TAG, "wifi_init_softap finished. SSID:'%s' password:'%s'",
    //          EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
}
void app_main(void)
{
    static httpd_handle_t server = NULL;

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_ap_test();

    /* Start the server for the first time */
    server = start_webserver();
}
