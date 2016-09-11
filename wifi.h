boolean AdminEnabled = true;    // Enable Admin Mode for a given Time

void ConfigureWifi()
{
  DBG_OUTPUT_PORT.println("Configuring Wifi");
  WiFi.begin (stringConfig.ssid.c_str(), stringConfig.password.c_str());
  if (!config.dhcp)
  {
    WiFi.config(IPAddress(config.IP[0],config.IP[1],config.IP[2],config.IP[3] ),  IPAddress(config.Gateway[0],config.Gateway[1],config.Gateway[2],config.Gateway[3] ) , IPAddress(config.Netmask[0],config.Netmask[1],config.Netmask[2],config.Netmask[3] ));
  }
  
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    DBG_OUTPUT_PORT.println("Connection Failed! Enabling Admin mode...");
    delay(20);
    AdminEnabled = true;
  }
  DBG_OUTPUT_PORT.println("Ready");
  DBG_OUTPUT_PORT.print("IP address: ");
  DBG_OUTPUT_PORT.println(WiFi.localIP());

}
