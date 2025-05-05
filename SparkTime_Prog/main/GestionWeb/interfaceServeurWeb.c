/*
    Nom:

    Description:

    Modification: -
    Création: 29/04/2025
    Auteur: Josée Girard
*/

#include "interfaceServeurWeb.h"
#include "esp_http_server.h"
#include "processusAffichageNeopixel.h"
#include "piloteServeurWeb.h"
#include <stdio.h>

const char *styleCSS =
    "<style>"
    "html, body {"
    "  height: 100%;"
    "  margin: 0;"
    "  padding: 0;"
    "  overflow-x: hidden;"
    "  overflow-y: auto;"
    "  font-family: Arial, sans-serif;"
    "  background-color: #f0f0f0;"
    "  display: flex;"
    "  justify-content: center;"
    "  align-items: center;"
    "  flex-direction: column;"
    "}"
    "h1 { font-size: 1.4em; margin-bottom: 0.5em; }"
    "h2 { font-size: 1.3em; margin-bottom: 0.5em; }"
    ".conteneurFormulaire {"
    "  background-color: white;"
    "  padding: 2em;"
    "  border-radius: 10px;"
    "  box-shadow: 0 0 10px rgba(0, 0, 0, 0.2);"
    "  width: 100%;"
    "  max-width: 400px;"
    "  text-align: center;"
    "  box-sizing: border-box;"
    "  display: flex;"
    "  flex-direction: column;"
    "  align-items: center;"
    "  justify-content: center;"
    "  margin: 40px;"
    "}"
    "select, input[type=text],input[type=password] {"
    "  width: 100%;"
    "  padding: 0.8em;"
    "  font-size: 1.2em;"
    "  margin-bottom: 1em;"
    "  border: 1px solid #ccc;"
    "  border-radius: 5px;"
    "  background-color: white;"
    "}"
    "input[type=button], input[type=submit] {"
    "  width: 100%;"
    "  padding: 1em;"
    "  font-size: 1.2em;"
    "  margin-bottom: 1em;"
    "  border: none;"
    "  border-radius: 5px;"
    "  background-color:rgb(52, 147, 235);"
    "  color: white;"
    "  cursor: pointer;"
    "  box-sizing: border-box;"
    "  font-weight: 600;"
    "}"
    "input[type=button]:hover, input[type=submit]:hover {"
    "  background-color:rgb(3, 109, 186);"
    "}"
    ".ligneHeure {"
    "  display: flex;"
    "  justify-content: space-between;"
    "  align-items: center;"
    "  width: 100%;"
    "  margin-bottom: 1em;"
    "}"
    ".ligneHeure label {"
    "  font-size: 1.2em;"
    "  margin-right: 1em;"
    "}"
    ".ligneHeure input[type=time] {"
    "  flex: 1;"
    "  padding: 0.8em;"
    "  font-size: 1.2em;"
    "  border: 1px solid #ccc;"
    "  border-radius: 5px;"
    "}"
    "@media (max-width: 480px) {"
    "  .conteneurFormulaire {"
    "    width: 90%;"
    "    padding: 1.5em;"
    "  }"
    "}"
    "</style>";

const char *optionsCouleur =
    "<option value=\"rouge\">Rouge</option>"
    "<option value=\"orange\">Orange</option>"
    "<option value=\"jaune\">Jaune</option>"
    "<option value=\"vert\">Vert</option>"
    "<option value=\"bleu\">Bleu</option>"
    "<option value=\"indigo\">Indigo</option>"
    "<option value=\"violet\">Violet</option>";

const char *jsPageSansWifi =
    "<script>"
    "document.getElementById(\"formMode\").onsubmit = function(e) {"
    "  e.preventDefault();"
    "  var mode = document.getElementById(\"mode\").value;"
    "  var xhr = new XMLHttpRequest();"
    "  xhr.open(\"GET\", \"/setModeSansWifi?mode=\" + mode, true);"
    "  xhr.send();"
    "};"

    "document.getElementById(\"formPerso\").onsubmit = function(e) {"
    "  e.preventDefault();"
    "  var heure = document.getElementById(\"heure\").value;"
    "  var couleurHeures = document.getElementById(\"couleurHeures\").value;"
    "  var couleurMinutes = document.getElementById(\"couleurMinutes\").value;"
    "  var couleurSecondes = document.getElementById(\"couleurSecondes\").value;"
    "  var affichageTemperature = document.getElementById(\"affichageTemperature\").checked;"
    " var affichageType = document.querySelector('input[name=\"affichageType\"]:checked').value;"
    "  var xhr = new XMLHttpRequest();"
    "  xhr.open(\"GET\", \"/setHorlogeSansWifi?heure=\" + encodeURIComponent(heure) +"
    "      \"&couleurHeures=\" + encodeURIComponent(couleurHeures) +"
    "      \"&couleurMinutes=\" + encodeURIComponent(couleurMinutes) +"
    "      \"&couleurSecondes=\" + encodeURIComponent(couleurSecondes) +"
    "      \"&affichageTemperature=\" + affichageTemperature +"
    "      \"&affichageType=\" + encodeURIComponent(affichageType), true);"
    "  xhr.send();"
    "};"
    "</script>";

const char *jsPageAvecWifi =
    "<script>"
    "document.getElementById(\"formPerso\").onsubmit = function(e) {"
    "e.preventDefault(); "

    "var villeActuelle = document.getElementById(\"villeActuelle\").value;"
    "var ville2e = document.getElementById(\"ville2e\").value;"
    "var couleurHeuresActuels = document.getElementById(\"couleurHeuresActuels\").value;"
    "var couleurMinutesActuels = document.getElementById(\"couleurMinutesActuels\").value;"
    "var couleurSecondes = document.getElementById(\"couleurSecondes\").value;"
    "var couleurHeures2e = document.getElementById(\"couleurHeures2e\").value;"
    "var couleurMinutes2e = document.getElementById(\"couleurMinutes2e\").value;"
    "var affichageTemperature = document.getElementById(\"affichageTemperature\").checked;"

    "var xhr = new XMLHttpRequest();"
    "xhr.open(\"GET\", \"/setHorlogeAvecWifi?villeActuelle=\" + encodeURIComponent(villeActuelle) + "
    "       \"&ville2e=\" + encodeURIComponent(ville2e) + "
    "       \"&couleurHeuresActuels=\" + encodeURIComponent(couleurHeuresActuels) +"
    "       \"&couleurMinutesActuels=\" + encodeURIComponent(couleurMinutesActuels) +"
    "       \"&couleurSecondes=\" + encodeURIComponent(couleurSecondes) +"
    "       \"&couleurHeures2e=\" + encodeURIComponent(couleurHeures2e) +"
    "       \"&couleurMinutes2e=\" + encodeURIComponent(couleurMinutes2e) +"
    "       \"&affichageTemperature=\" + affichageTemperature, true);"
    "  xhr.send(); "
    "};"

    "document.getElementById(\"formMode\").onsubmit = function(e) {"
    "  e.preventDefault();"
    "  var mode = document.getElementById(\"mode\").value;"
    "  var xhr = new XMLHttpRequest();"
    "  xhr.open(\"GET\", \"/setModeAvecWifi?mode=\" + mode, true);"
    "  xhr.send();"
    "};"
    "</script>";

esp_err_t pageAccueilHandler(httpd_req_t *req)
{
    static char pageAccueil[8192];
    snprintf(pageAccueil, sizeof(pageAccueil),
             "<html>"
             "<head>"
             "<meta charset=\"UTF-8\">"
             "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
             "%s"
             "</head>"
             "<body>"
             "<div class=\"conteneurFormulaire\">"

             "<h1>SparkTime - Connexion</h1>"

             "<h2>Connexion au Wi-Fi</h2>"
             "<form id=\"formWifi\">"
             "<input type=\"text\" id=\"ssidConnectWifi\" name=\"ssid\" placeholder=\"Nom du réseau (SSID)\" required><br>"
             "<input type=\"password\" id=\"mdpConnectWifi\" name=\"password\" placeholder=\"Mot de passe\" required><br>"
             "<input type=\"submit\" value=\"Connexion avec WiFi\">"
             "</form>"

             "<hr style=\"margin: 2em 0; width: 100%%; border: none; border-top: 2px solid #ccc;\">"

             "<input type=\"button\" value=\"Connexion sans WiFi\" onclick=\"location.href='/pageSansWifi'\">"

             "<script>"
             "document.getElementById(\"formWifi\").onsubmit = function(e) {"
             "  e.preventDefault();"
             "  var ssidWifi = document.getElementById(\"ssidConnectWifi\").value;"
             "  var mdpWifi = document.getElementById(\"mdpConnectWifi\").value;"
             "  var xhr = new XMLHttpRequest();"
             "  xhr.open(\"GET\", \"/connectWifi?ssid=\" + encodeURIComponent(ssidWifi) + \"&password=\" + encodeURIComponent(mdpWifi), true);"
             "  xhr.send();"
             "};"
             "</script>"

             "</div>"
             "</body>"
             "</html>",
             styleCSS);
    httpd_resp_send(req, pageAccueil, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t pagePersonnalisationSansWifiHandler(httpd_req_t *req)
{
    static char pageSansWifi[8192];

    snprintf(pageSansWifi, sizeof(pageSansWifi),
             "<html>"
             "<head>"
             "<meta charset=\"UTF-8\">"
             "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
             "%s"
             "</head>"
             "<body>"
             "<div class=\"conteneurFormulaire\">"

             "<h1>SparkTime - version sans wifi</h1>"

             "<h2>Personnalisation de l'horloge</h2><br>"
             "<form id=\"formPerso\">"
             "<div class=\"ligneHeure\">"
             "<label for=\"heure\">Heure actuelle:</label>"
             "<input type=\"time\" id=\"heure\" name=\"heure\" required>"
             "</div>"
             "<select name=\"couleurHeures\" id=\"couleurHeures\" required>"
             "<option value=\"\" disabled selected hidden>— Couleur des heures —</option>"
             "%s"
             "</select>"

             "<select name=\"couleurMinutes\" id=\"couleurMinutes\" required>"
             "<option value=\"\" disabled selected hidden>— Couleur des minutes —</option>"
             "%s"
             "</select>"

             "<select name=\"couleurSecondes\" id=\"couleurSecondes\" required>"
             "<option value=\"\" disabled selected hidden>— Couleur des secondes —</option>"
             "%s"
             "</select>"

             "<div class=\"ligneHeure\">"
             "<label for=\"affichageTemperature\">Affichage de température :</label>"
             "<input type=\"checkbox\" id=\"affichageTemperature\" name=\"affichageTemperature\"><br>"
             "</div>"

             "<fieldset>"
             "   <legend>Type d'affichage :</legend>"
             "   <label for=\"affichageRegulier\">Affichage régulier</label>"
             "   <input type=\"radio\" id=\"affichageRegulier\" name=\"affichageType\" value=\"regulier\" checked><br>"
             "   <label for=\"affichageContinu\">Affichage continu</label>"
             "   <input type=\"radio\" id=\"affichageContinu\" name=\"affichageType\" value=\"continu\"><br>"
             "</fieldset>"
             "<input type=\"submit\" value=\"Personnaliser\">"
             "</form>"

             "<hr style=\"margin: 2em 0; width: 100%%; border: none; border-top: 2px solid #ccc;\">"

             "<h2>Tests</h2>"
             "<form id=\"formMode\">"
             "<select id=\"mode\" name=\"mode\">"
             "<option value=\"\" disabled selected>Sélectionner le mode</option>"
             "<option value=\"2\">test de leds</option>"
             "<option value=\"3\">arc-en-ciel</option>"
             "<option value=\"4\">rien</option>"
             "</select><br>"
             "<input type=\"submit\" value=\"GO\">"
             "</form>"
             "<hr style=\"margin: 2em 0; width: 100%%; border: none; border-top: 2px solid #ccc;\">"
             "<input type=\"button\" value=\"Accueil\" onclick=\"location.href='/'\">"

             "%s"

             "</div>"
             "</body>"
             "</html>",
             styleCSS, optionsCouleur, optionsCouleur, optionsCouleur, jsPageSansWifi);
    httpd_resp_send(req, pageSansWifi, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t pagePersonnalisationAvecWifiHandler(httpd_req_t *req)
{
    static char pageAvecWifi[8192];

    snprintf(pageAvecWifi, sizeof(pageAvecWifi),
             "<html>"
             "<head>"
             "<meta charset=\"UTF-8\">"
             "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
             "%s"
             "</head>"
             "<body>"
             "<div class=\"conteneurFormulaire\">"

             "<h1>SparkTime - version avec wifi</h1>"

             "<h2>Personnalisation de l'horloge</h2><br>"
             "<form id=\"formPerso\">"

             "<select name=\"villeActuelle\" id=\"villeActuelle\" required>"
             "<option value=\"\" disabled selected hidden>— Choisir la ville actuelle —</option>"
             "<option value=\"Montreal\">Montréal</option>"
             "<option value=\"Paris\">Paris</option>"
             "<option value=\"Tokyo\">Tokyo</option>"
             "<option value=\"NewYork\">New York</option>"
             "</select>"

             "<select name=\"ville2e\" id=\"ville2e\" required>"
             "<option value=\"\" disabled selected hidden>— Choisir la 2<sup>e</sup> ville —</option>"
             "<option value=\"Montreal\">Montréal</option>"
             "<option value=\"Paris\">Paris</option>"
             "<option value=\"Tokyo\">Tokyo</option>"
             "<option value=\"NewYork\">New York</option>"
             "</select>"

             "<p>Couleurs pour l'heure de la ville actuelle:</p>"

             "<select name=\"couleurHeuresActuels\" id=\"couleurHeuresActuels\" required>"
             "<option value=\"\" disabled selected hidden>— Couleur des heures —</option>"
             "%s"
             "</select>"

             "<select name=\"couleurMinutesActuels\" id=\"couleurMinutesActuels\" >"
             "<option value=\"\" disabled selected hidden>— Couleur des minutes —</option>"
             "%s"
             "</select>"

             "<select name=\"couleurSecondes\" id=\"couleurSecondes\" required>"
             "<option value=\"\" disabled selected hidden>— Couleur des secondes —</option>"
             "%s"
             "</select>"

             "<p>Couleur de l'heure de la 2<sup>e</sup> ville:</p>"
             "<p><small>*La couleur des minutes sera seulement appliqué si le fuseau horaire de la 2<sup>e</sup> ville n'a pas les mêmes minutes que la ville actuelle.</small></p>"

             "<select name=\"couleurHeures2e\" id=\"couleurHeures2e\" required>"
             "<option value=\"\" disabled selected hidden>— Couleur des heures —</option>"
             "%s"
             "</select>"

             "<select name=\"couleurMinutes2e\" id=\"couleurMinutes2e\" required>"
             "<option value=\"\" disabled selected hidden>— Couleur des minutes —</option>"
             "%s"
             "</select>"

             "<div class=\"ligneHeure\">"
             "<label for=\"affichageTemperature\">Affichage de température :</label>"
             "<input type=\"checkbox\" id=\"affichageTemperature\" name=\"affichageTemperature\"><br>"
             "</div>"

             "<input type=\"submit\" value=\"Personnaliser\">"
             "</form>"

             "<hr style=\"margin: 2em 0; width: 100%%; border: none; border-top: 2px solid #ccc;\">"

             "<h2>Tests</h2>"
             "<form id=\"formMode\">"
             "<select id=\"mode\" name=\"mode\">"
             "<option value=\"\" disabled selected>Sélectionner le mode</option>"
             "<option value=\"2\">test de leds</option>"
             "<option value=\"3\">arc-en-ciel</option>"
             "<option value=\"4\">rien</option>"
             "</select><br>"
             "<input type=\"submit\" value=\"GO\">"
             "</form>"
             "<hr style=\"margin: 2em 0; width: 100%%; border: none; border-top: 2px solid #ccc;\">"
             "<input type=\"button\" value=\"Accueil\" onclick=\"location.href='/'\">"

             "%s"

             "</div>"
             "</body>"
             "</html>",
             styleCSS, optionsCouleur, optionsCouleur, optionsCouleur, optionsCouleur, optionsCouleur, jsPageAvecWifi);
    httpd_resp_send(req, pageAvecWifi, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
