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
#include <string.h>

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
    "<option value=\"jaune-vert\">Jaune-vert</option>"
    "<option value=\"vert\">Vert</option>"
    "<option value=\"vert-bleu\">Vert-bleu</option>"
    "<option value=\"turquoise\">Turquoise</option>"
    "<option value=\"bleu\">Bleu</option>"
    "<option value=\"indigo\">Indigo</option>"
    "<option value=\"violet\">Violet</option>"
    "<option value=\"mauve\">Mauve</option>"
    "<option value=\"rose\">Rose</option>"
    "<option value=\"blanc\">Blanc</option>";

const char *jsPageSansWifi =
    "<script>"
    "document.getElementById(\"formMode\").onsubmit = function(e) {"
    "  e.preventDefault();"
    "  var mode = document.getElementById(\"mode\").value;"
    "  if (!mode) {"
    "    alert('Veuillez sélectionner un mode');"
    "    return false;"
    "  }"
    "  var xhr = new XMLHttpRequest();"
    "  xhr.open(\"GET\", \"/setModeSansWifi?mode=\" + mode, true);"
    "  xhr.onload = function() {"
    "    if (xhr.status === 200) {"
    "      console.log('Mode activé');"
    "    } else {"
    "      alert('Erreur lors du changement de mode');"
    "    }"
    "  };"
    "  xhr.send();"
    "};"

    "document.getElementById(\"formPerso\").onsubmit = function(e) {"
    "  e.preventDefault();"
    "  var heure = document.getElementById(\"heure\").value;"
    "  var couleurHeures = document.getElementById(\"couleurHeures\").value;"
    "  var couleurMinutes = document.getElementById(\"couleurMinutes\").value;"
    "  var couleurSecondes = document.getElementById(\"couleurSecondes\").value;"
    "  var affichageType = document.querySelector('input[name=\"affichageType\"]:checked').value;"
    "  var affichageTemperature = document.getElementById(\"affichageTemperature\").checked;"

    "  if (!heure || !couleurHeures || !couleurMinutes || !couleurSecondes) {"
    "    alert('Veuillez remplir tous les champs requis');"
    "    return false;"
    "  }"

    "  var xhr = new XMLHttpRequest();"
    "  xhr.open(\"GET\", \"/setHorlogeSansWifi?heure=\" + encodeURIComponent(heure) +"
    "    \"&couleurHeures=\" + encodeURIComponent(couleurHeures) +"
    "    \"&couleurMinutes=\" + encodeURIComponent(couleurMinutes) +"
    "    \"&couleurSecondes=\" + encodeURIComponent(couleurSecondes) +"
    "    \"&affichageTemperature=\" + affichageTemperature +"
    "    \"&affichageType=\" + encodeURIComponent(affichageType), true);"

    "  xhr.onload = function() {"
    "    if (xhr.status === 200) {"
    "      console.log('Configuration appliquée');"
    "    } else {"
    "      alert('Erreur lors de la configuration');"
    "    }"
    "  };"
    "  xhr.send();"
    "};"
    "</script>";

const char *jsPageAvecWifi =
    "<script>"
    "document.getElementById(\"formMode\").onsubmit = function(e) {"
    "  e.preventDefault();"
    "  var mode = document.getElementById(\"mode\").value;"
    "  if (!mode) {"
    "    alert('Veuillez sélectionner un mode');"
    "    return false;"
    "  }"
    "  var xhr = new XMLHttpRequest();"
    "  xhr.open(\"GET\", \"/setModeAvecWifi?mode=\" + mode, true);"
    "  xhr.onload = function() {"
    "    if (xhr.status === 200) {"
    "      console.log('Mode activé');"
    "    } else {"
    "      alert('Erreur lors du changement de mode');"
    "    }"
    "  };"
    "  xhr.send();"
    "};"

    "document.getElementById(\"formPerso\").onsubmit = function(e) {"
    "  e.preventDefault();"
    "  var p = {"
    "    v: document.getElementById(\"ville2e\").value,"
    "    h: document.getElementById(\"couleurHeuresActuels\").value,"
    "    m: document.getElementById(\"couleurMinutesActuels\").value,"
    "    s: document.getElementById(\"couleurSecondes\").value,"
    "    h2: document.getElementById(\"couleurHeures2e\").value"
    "  };"
    "  if (!p.v || !p.h || !p.m || !p.s || !p.h2) {"
    "    alert('Veuillez remplir tous les champs requis');"
    "    return false;"
    "  }"
    "  var xhr = new XMLHttpRequest();"
    "  xhr.open(\"GET\", \"/setHorlogeAvecWifi?\" +"
    "    \"v=\" + encodeURIComponent(p.v) +"
    "    \"&h=\" + encodeURIComponent(p.h) +"
    "    \"&m=\" + encodeURIComponent(p.m) +"
    "    \"&s=\" + encodeURIComponent(p.s) +"
    "    \"&h2=\" + encodeURIComponent(p.h2), true);"  // Removed trailing semicolon

    "  xhr.onload = function() {"
    "    if (xhr.status === 200) {"
    "      console.log('Configuration appliquée');"
    "    } else {"
    "      alert('Erreur lors de la configuration');"
    "    }"
    "  };"
    "  xhr.send();"
    "  return false;"
    "};"
    "</script>";

const char *htmlAvecWifiDebut =
    "<html>"
    "<head>"
    "<meta charset=\"UTF-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";

const char *htmlAvecWifiContenu =
    "<div class=\"conteneurFormulaire\">"
    "<h1>SparkTime - version avec wifi</h1>"
    "<h2>Personnalisation de l'horloge</h2><br>"
    "<form id=\"formPerso\">"

    //"<select name=\"villeActuelle\" id=\"villeActuelle\" required>"

    "</select>"
    "<select name=\"ville2e\" id=\"ville2e\" required>"
    "<option value=\"\" disabled selected hidden>— Choisir la 2<sup>e</sup> ville —</option>"
    "<option value=\"-12\">UTC-12: Inhabité</option>"
    "<option value=\"-11\">UTC-11: Samoa américaines</option>"
    "<option value=\"-10\">UTC-10: Îles Cook</option>"
    "<option value=\"-9\">UTC-9: Îles Gambier</option>"
    "<option value=\"-8\">UTC-8: Alaska (USA)</option>"
    "<option value=\"-7\">UTC-7: Yukon (Canada)</option>"
    "<option value=\"-6\">UTC-6: Costa Rica</option>"
    "<option value=\"-5\">UTC-5: Panama</option>"
    "<option value=\"\"disabled hidden>UTC-4: Québec (Canada)</option>" // Disabled
    "<option value=\"-3\">UTC-3: Argentine</option>"
    "<option value=\"-2\">UTC-2: Saint-Pierre-et-Miquelon (France)</option>"
    "<option value=\"-1\">UTC-1: Cap-Vert</option>"
    "<option value=\"0\">UTC 0: Islande</option>"
    "<option value=\"+1\">UTC+1: Algérie</option>"
    "<option value=\"+2\">UTC+2: Italie</option>"
    "<option value=\"+3\">UTC+3: Kenya</option>"
    "<option value=\"+4\">UTC+4: Arménie</option>"
    "<option value=\"+5\">UTC+5: Turkménistan </option>"
    "<option value=\"+6\">UTC+6: Bangladesh</option>"
    "<option value=\"+7\">UTC+7: Cambodge</option>"
    "<option value=\"\"disabled hidden>UTC+8: Philippines</option>"
    "<option value=\"+9\">UTC+9: Japon</option>"
    "<option value=\"+10\">UTC+10: Papouasie-Nouvelle-Guinée</option>"
    "<option value=\"+11\">UTC+11: Îles Salomon</option>"
    "<option value=\"+12\">UTC+12: Fidji</option>"
    "<option value=\"+13\">UTC+13: Samoa</option>"
    "<option value=\"+14\">UTC+14: Îles de la Ligne (Kiribati)</option>"
    "</select>"
    "<p>Couleurs pour l'heure de la ville actuelle:</p>"
    "<select name=\"couleurHeuresActuels\" id=\"couleurHeuresActuels\" required>"
    "<option value=\"\" disabled selected hidden>— Couleur des heures —</option>";

const char *htmlAvecWifiCouleurs2 =
    "</select>"
    "<select name=\"couleurMinutesActuels\" id=\"couleurMinutesActuels\" >"
    "<option value=\"\" disabled selected hidden>— Couleur des minutes —</option>";

const char *htmlAvecWifiCouleurs3 =
    "</select>"
    "<select name=\"couleurSecondes\" id=\"couleurSecondes\" required>"
    "<option value=\"\" disabled selected hidden>— Couleur des secondes —</option>";

const char *htmlAvecWifiCouleurs4 =
    "</select>"
    "<p>Couleur de l'heure de la 2<sup>e</sup> ville:</p>"
    "<select name=\"couleurHeures2e\" id=\"couleurHeures2e\" required>"
    "<option value=\"\" disabled selected hidden>— Couleur des heures —</option>";

/*const char *htmlAvecWifiCouleurs5 =
    "</select>"
    "<select name=\"couleurMinutes2e\" id=\"couleurMinutes2e\" required>"
    "<option value=\"\" disabled selected hidden>— Couleur des minutes —</option>";*/

const char *htmlAvecWifiFin =
    "</select>"
    /*"<div class=\"ligneHeure\">"
    "<label for=\"affichageTemperature\">Affichage de température :</label>"
    "<input type=\"checkbox\" id=\"affichageTemperature\" name=\"affichageTemperature\"><br>"
    "</div>"
    "<input type=\"hidden\" name=\"affichageType\" value=\"regulier\">"*/
    "<input type=\"submit\" value=\"Personnaliser\">"
    "</form>"
    "<hr style=\"margin: 2em 0; width: 100%; border: none; border-top: 2px solid #ccc;\">"
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
    "<hr style=\"margin: 2em 0; width: 100%; border: none; border-top: 2px solid #ccc;\">"
    "<input type=\"button\" value=\"Accueil\" onclick=\"location.href='/'\">"
    "</div>"
    "</body>"
    "</html>";

const char *htmlSansWifiDebut =
    "<html>"
    "<head>"
    "<meta charset=\"UTF-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";

const char *htmlSansWifiContenu =
    "<div class=\"conteneurFormulaire\">"
    "<h1>SparkTime - version sans wifi</h1>"
    "<h2>Personnalisation de l'horloge</h2><br>"
    "<form id=\"formPerso\">"
    "<div class=\"ligneHeure\">"
    "<label for=\"heure\">Heure actuelle:</label>"
    "<input type=\"time\" id=\"heure\" name=\"heure\" required>"
    "</div>"
    "<select name=\"couleurHeures\" id=\"couleurHeures\" required>"
    "<option value=\"\" disabled selected hidden>— Couleur des heures —</option>";

const char *htmlSansWifiCouleurs2 =
    "</select>"
    "<select name=\"couleurMinutes\" id=\"couleurMinutes\" required>"
    "<option value=\"\" disabled selected hidden>— Couleur des minutes —</option>";

const char *htmlSansWifiCouleurs3 =
    "</select>"
    "<select name=\"couleurSecondes\" id=\"couleurSecondes\" required>"
    "<option value=\"\" disabled selected hidden>— Couleur des secondes —</option>";

const char *htmlSansWifiFin =
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
    "<hr style=\"margin: 2em 0; width: 100%; border: none; border-top: 2px solid #ccc;\">"
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
    "<hr style=\"margin: 2em 0; width: 100%; border: none; border-top: 2px solid #ccc;\">"
    "<input type=\"button\" value=\"Accueil\" onclick=\"location.href='/'\">"
    "</div>"
    "</body>"
    "</html>";

const char *htmlAccueilDebut =
    "<html>"
    "<head>"
    "<meta charset=\"UTF-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";

const char *htmlAccueilContenu =
    "<div class=\"conteneurFormulaire\">"
    "<h1>SparkTime - Connexion</h1>"
    "<h2>Mode de connexion</h2>"
    "<input type=\"button\" value=\"Connexion avec WiFi\" onclick=\"connecterWifi()\">"
    "<hr style=\"margin: 2em 0; width: 100%; border: none; border-top: 2px solid #ccc;\">"
    "<input type=\"button\" value=\"Connexion sans WiFi\" onclick=\"location.href='/pageSansWifi'\">"
    "<hr style=\"margin: 2em 0; width: 100%; border: none; border-top: 2px solid #ccc;\">"
    "<input type=\"button\" value=\"Redémarrer\" onclick=\"redemarrerESP()\" style=\"background-color:rgb(154, 28, 28);\">";

const char *htmlAccueilScript =
    "<script>"
    "function connecterWifi() {"
    "  var xhr = new XMLHttpRequest();"
    "  xhr.open('GET', '/connectWifi', true);"
    "  xhr.onload = function() {"
    "    if (xhr.status === 200) {"
    "      setTimeout(function() { window.location.href = '/pageAvecWifi'; }, 2000);"
    "    }"
    "  };"
    "  xhr.send();"
    "}"
    "function redemarrerESP() {"
    "  if(confirm('Voulez-vous vraiment redémarrer le système?')) {"
    "    console.log('Envoi de la requête de redémarrage');"
    "    var xhr = new XMLHttpRequest();"
    "    xhr.open('GET', '/redemarrer', true);"
    "    xhr.onload = function() {"
    "      console.log('Réponse reçue:', xhr.status);"
    "      if (xhr.status === 200) {"
    "        document.body.innerHTML = '<div style=\"text-align:center;padding:20px;\">"
    "          <h2>Redémarrage en cours...</h2>"
    "          <p>Veuillez patienter 15 secondes</p>"
    "          <p>La page se rechargera automatiquement</p></div>';"
    "        setTimeout(function() {"
    "          window.location.href = '/';"
    "        }, 15000);"
    "      } else {"
    "        alert('Erreur lors du redémarrage: ' + xhr.status);"
    "      }"
    "    };"
    "    xhr.onerror = function() {"
    "      console.log('Erreur de connexion');"
    "      alert('Erreur de connexion au serveur');"
    "    };"
    "    xhr.send();"
    "  }"
    "}"
    "</script>"
    "</div>"
    "</body>"
    "</html>";

esp_err_t pageAccueilHandler(httpd_req_t *req)
{
    static char pageAccueil[8192] = {0};

    // Réinitialiser le buffer
    memset(pageAccueil, 0, sizeof(pageAccueil));

    // Concaténer toutes les parties
    strcat(pageAccueil, htmlAccueilDebut);
    strcat(pageAccueil, styleCSS);
    strcat(pageAccueil, "</head><body>");
    strcat(pageAccueil, htmlAccueilContenu);
    strcat(pageAccueil, htmlAccueilScript);

    httpd_resp_send(req, pageAccueil, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t pagePersonnalisationSansWifiHandler(httpd_req_t *req)
{
    static char pageSansWifi[8192] = {0};

    // Réinitialiser le buffer
    memset(pageSansWifi, 0, sizeof(pageSansWifi));

    // Concaténer toutes les parties
    strcat(pageSansWifi, htmlSansWifiDebut);
    strcat(pageSansWifi, styleCSS);
    strcat(pageSansWifi, "</head><body>");
    strcat(pageSansWifi, htmlSansWifiContenu);
    strcat(pageSansWifi, optionsCouleur);
    strcat(pageSansWifi, htmlSansWifiCouleurs2);
    strcat(pageSansWifi, optionsCouleur);
    strcat(pageSansWifi, htmlSansWifiCouleurs3);
    strcat(pageSansWifi, optionsCouleur);
    strcat(pageSansWifi, htmlSansWifiFin);
    strcat(pageSansWifi, jsPageSansWifi);

    httpd_resp_send(req, pageSansWifi, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t pagePersonnalisationAvecWifiHandler(httpd_req_t *req)
{
    static char pageAvecWifi[8192] = {0};

    // Réinitialiser le buffer
    memset(pageAvecWifi, 0, sizeof(pageAvecWifi));

    // Concaténer toutes les parties
    strcat(pageAvecWifi, htmlAvecWifiDebut);
    strcat(pageAvecWifi, styleCSS);
    strcat(pageAvecWifi, "</head><body>");
    strcat(pageAvecWifi, htmlAvecWifiContenu);
    strcat(pageAvecWifi, optionsCouleur);
    strcat(pageAvecWifi, htmlAvecWifiCouleurs2);
    strcat(pageAvecWifi, optionsCouleur);
    strcat(pageAvecWifi, htmlAvecWifiCouleurs3);
    strcat(pageAvecWifi, optionsCouleur);
    strcat(pageAvecWifi, htmlAvecWifiCouleurs4);
    strcat(pageAvecWifi, optionsCouleur);
    // strcat(pageAvecWifi, htmlAvecWifiCouleurs5);
    // strcat(pageAvecWifi, optionsCouleur);
    strcat(pageAvecWifi, htmlAvecWifiFin);
    strcat(pageAvecWifi, jsPageAvecWifi);

    httpd_resp_send(req, pageAvecWifi, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}