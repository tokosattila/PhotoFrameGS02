#ifndef NETINFO_COMMAND_CLASS
#define NETINFO_COMMAND_CLASS

#include <App/Telnet/Command.h>

namespace App {

  class NetInfoCommand_ : public Command_ {
    public:
      const char *GetName() const override { 
        return "netinfo"; 
      }
      bool Execute(const char *tArguments, WiFiClient &tClient) override {
        wifi_mode_t tMode = WiFi.getMode();
        bool tIsAP = (tMode == WIFI_AP || tMode == WIFI_AP_STA);
        tClient.print(F("\r\n  Mode: "));
        tClient.println(tIsAP ? "AP" : "STA");
        if (tIsAP) {
          tClient.print(F("  SSID: "));
          tClient.println(WiFi.softAPSSID());
          tClient.print(F("  Connected clients: "));
          tClient.println(WiFi.softAPgetStationNum());
        } else {
          tClient.print(F("  SSID: "));
          tClient.println(WiFi.SSID());
          tClient.print(F("  RSSI: "));
          tClient.print(WiFi.RSSI());
          tClient.println(F(" dBm"));
        }
        tClient.println();
        tClient.print(F("  IP address: "));
        tClient.println(tIsAP ? WiFi.softAPIP() : WiFi.localIP());
        tClient.print(F("  Subnet mask: "));
        tClient.println(tIsAP ? WiFi.softAPSubnetMask() : WiFi.subnetMask());
        tClient.print(F("  Gateway: "));
        tClient.println(tIsAP ? WiFi.softAPIP() : WiFi.gatewayIP());
        if (!tIsAP) {
          tClient.print(F("  DNS 1: "));
          tClient.println(WiFi.dnsIP(0));
          tClient.print(F("  DNS 2: "));
          tClient.println(WiFi.dnsIP(1));
        }
        tClient.print(F("  MAC address: "));
        tClient.println(WiFi.macAddress());
        tClient.print(F("\r\n"));
        return true;
      }
      const char *Help() const override {
        return "netinfo                           " COLOR_YELLOW "- show network info" COLOR_WHITE;
      }
  };

}

#endif
