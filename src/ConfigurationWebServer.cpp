#include "ConfigurationWebServer.h"

#include <ESPmDNS.h>

void ConfigurationWebServer::Initialise() {
    MDNS.begin("microradar"); // hostname resolution for ip

    // Serve the config page
    server.on("/", HTTP_GET,
        [&](AsyncWebServerRequest* request) {
            Serial.println("[GET] Handling request to config web server...");

            prefs.begin("config", true);  // true = read only
            String latitude = prefs.getString("latitude", "");
            String longitude = prefs.getString("longitude", "");
            String radius = prefs.getString("radius", "1.0");
            String openskyClientId = prefs.getString("opensky-id", "");
            String openskyClientSecret = prefs.getString("opensky-secret", "");
            prefs.end();

            std::fill(openskyClientSecret.begin(), openskyClientSecret.end(), '*'); // replace secret string with asterisks

            String html = R"(
                <html>
                    <head>
                        <title>Configure Micro Radar</title>
                        <script src="https://cdn.jsdelivr.net/npm/@tailwindcss/browser@4.3.0"></script>
                    </head>
                    <body class="font-mono bg-gray-900 text-green-500">
                        <fieldset class="border border-green-500 p-5 m-10 inline-block">
                            <legend class="ml-5 px-2">Configure Micro Radar</legend>
                            <form id="cfg" action='/save' method='POST' class="flex flex-col gap-2">
                                <div class="flex gap-5">
                                    <label class="flex items-center gap-2">
                                        Latitude:
                                        <input name='latitude' type="number" min="-90" step="0.000001" max="90" value=')" + latitude + R"(' class="border border-green-500">
                                    </label>
                                    <label class="flex items-center gap-2">
                                        Longitude:
                                        <input name='longitude' type="number" min="-180" step="0.000001" max="180" value=')" + longitude + R"(' class="border border-green-500">
                                    </label>
                                </div>
                                <label class="flex items-center gap-2">
                                    Radius (in &deg;):
                                    <input name='radius' type="number" min="0" step="0.000001" max="2" value=')" + radius + R"(' class="flex-1 border border-green-500">
                                </label>
                                <label class="flex items-center gap-2">
                                    OpenSkyAPI Client ID:
                                    <input name='opensky-id' value=')" + openskyClientId + R"(' class="flex-1 border border-green-500">
                                </label>
                                <label class="flex items-center gap-2">
                                    OpenSkyAPI Client Secret:
                                    <input name='opensky-secret' value=')" + openskyClientSecret + R"(' class="flex-1 border border-green-500">
                                </label>

                                <input type='submit' value='Save' class="bg-green-500 text-black mt-4 px-2 self-start cursor-pointer">
                            </form>
                        </fieldset>

                        <div id="result" class="px-10"></div>

                        <script>
                            document.getElementById('cfg').addEventListener('submit', function(e) {
                                e.preventDefault();
                                fetch(this.action, { method: 'POST', body: new FormData(this) })
                                    .then(r => r.text())
                                    .then(html => document.getElementById('result').innerHTML = html);
                            });
                        </script>
                    </body>
                </html>
                )";

            request->send(200, "text/html", html);
        }
    );

    // Handle form submission
    server.on("/save", HTTP_POST,
        [&](AsyncWebServerRequest* request) {
            Serial.println("[POST] Handling form submission to config web server...");

            prefs.begin("config", false);
            if (request->hasParam("latitude", true))
                prefs.putString("latitude", request->getParam("latitude", true)->value());
            if (request->hasParam("longitude", true))
                prefs.putString("longitude", request->getParam("longitude", true)->value());
            if (request->hasParam("radius", true))
                prefs.putString("radius", request->getParam("radius", true)->value());
            if (request->hasParam("opensky-id", true))
                prefs.putString("opensky-id", request->getParam("opensky-id", true)->value());
            if (request->hasParam("opensky-secret", true)) {
                String secret = request->getParam("opensky-secret", true)->value();
                if (secret.indexOf('*') == -1) prefs.putString("opensky-secret", secret); // avoid overwriting with obscured string
            }
            prefs.end();

            request->send(200, "text/html", "Saved - restarting device...");

            ESP.restart();
        }
    );

    server.begin();
}

String ConfigurationWebServer::GetStoredString(const char* key)
{
    prefs.begin("config", true);
    String value = prefs.getString(key, "");
    prefs.end();
    return value;
}
