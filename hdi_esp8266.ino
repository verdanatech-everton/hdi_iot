#define ON_LED D1
#define REQUEST_LED D2
#define CREATE_BUTTON D5
#define FOLLOWUP_BUTTON D6
#define CLOSE_BUTTON D7

#include <glpi_esp8266.h>
#include <ArduinoJson.h>

// CONFIGURAÇÂO
char SSID[] = "VTC_Visitante";
char NETKEY[] = "G@taV3!d&";
char tokenIot[] = "q*KiMHVK";
char tokenClient[] = "30a27cb57a6f0675d59a36fdb84c741b";
char assetName[] = "hdi_esp8266";
long EVENT_ID = 778;
int attempts = 50;

String lastTicketId = "";
int createButtonState = 0;
int followupButtonState = 0;
int closeButtonState = 0;

GlpiIot invasion(tokenIot, tokenClient);

void setup()
{
  pinMode(CREATE_BUTTON, INPUT);
  pinMode(FOLLOWUP_BUTTON, INPUT);
  pinMode(CLOSE_BUTTON, INPUT);
  pinMode(ON_LED, OUTPUT);
  pinMode(REQUEST_LED, OUTPUT);

  Serial.begin(9600);

  wifiConect();
  printNetworkData();
  delay(1000);
  invasion.SetEventIdReq(EVENT_ID);
}

void loop()
{
  createButtonState = digitalRead(CREATE_BUTTON);
  followupButtonState = digitalRead(FOLLOWUP_BUTTON);
  closeButtonState = digitalRead(CLOSE_BUTTON);

  if (createButtonState == HIGH)
  {
    Serial.println("Criação pressionado");
    digitalWrite(REQUEST_LED, HIGH);
    String ticketCreatedResponse = invasion.NewTicketRequest("HDI Brasil", "IoT", 3, "Criação de chamado", assetName);
    String ticketCreated = getResponse(ticketCreatedResponse);

    if (ticketCreated == lastTicketId)
    {
      Serial.println("Seu ticket já existe. Adicionando acompanhamento em: " + lastTicketId);
    }
    else
    {
      Serial.println("Ticket criado com id: " + ticketCreated);
      lastTicketId = ticketCreated;
    }
    digitalWrite(REQUEST_LED, LOW);
  }
  else if (followupButtonState == HIGH)
  {
    Serial.println("Acompanhamento pressionado");
    digitalWrite(REQUEST_LED, HIGH);
    if (lastTicketId != "")
    {
      Serial.println("Adicionando acompanhamento para o ticket: " + lastTicketId);
      invasion.FollowupTicket(lastTicketId, "HDI Brasil");
    }
    else
    {
      Serial.println("Crie um ticket antes de adicionar acompanhamento");
    }
    digitalWrite(REQUEST_LED, LOW);
  }
  else if (closeButtonState == HIGH)
  {
    Serial.println("Fechamento pressionado");
    digitalWrite(REQUEST_LED, HIGH);
    if (lastTicketId != "")
    {
      Serial.println("Fechando o ticket: " + lastTicketId);
      invasion.SolutionTicket(lastTicketId, "HDI Brasil");
      lastTicketId = "";
    }
    else
    {
      Serial.println("Crie um ticket antes de fechá-lo");
    }
    digitalWrite(REQUEST_LED, LOW);
  }

  delay(500);
}

String getResponse(String jsonString)
{
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, jsonString);
  if (error)
  {
    Serial.print(F("Erro na resposta do servidor"));
    Serial.println(error.c_str());
    return "";
  }
  return (String)doc["result"];
}

void printNetworkData()
{
  digitalWrite(ON_LED, HIGH);
  Serial.println();
  Serial.println("- - - - - - - - - - - - - - - - - - - - - - - -");
  Serial.println("Hostname: " + (String)assetName);

  Serial.println("Connected in: " + (String)SSID);

  Serial.print("My address IP: ");
  Serial.println(WiFi.localIP());

  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.println("- - - - - - - - - - - - - - - - - - - - - - - - ");
}

void wifiConect()
{

  delay(2000);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.begin(SSID, NETKEY);

  WiFi.hostname(assetName);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    attempts--;

    if (attempts == 0)
    {
      Serial.println();
      Serial.println("Time-out. Check your settings and try again.");
      delay(1000);
    }
  }
}
