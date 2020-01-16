#include <ESP8266WiFi.h>
#include <WiFiServer.h>
#include <PubSubClient.h>

WiFiServer sv(555);//Cria o objeto servidor na porta 555
WiFiClient cl;//Cria o objeto cliente.
WiFiClient clmqtt;//Cria o objeto cliente do mqtt.
PubSubClient client(clmqtt);

bool servidor = false; //variavel controla o laco de receber informacoes pela tcp
bool wifi = false; //variavel controla o estado do WiFi
String ssid = "";// variavel responsavel por armazenar o usuario do WiFi
String password = "";// variavel responsavel por armazenar a senha do WiFi
String mqtt_server = "";// variavel responsavel por armazenar o ip(dns) do broker
String mqttUser = "";//variavel responsavel por armazenar o usuario do broker
String mqttPassword = "";//variavel responsavel por armazenar a senha do broker 
int mqttPort;//variavel responsavel por armazenar a porta do broker 

//Bloco do Wifi

//função que configura o WiFi a ser usado para o protocolo MQTT(true - conectou false não-conectou)
//tempo limite para conectar 30s
bool setup_wifi(){
   int vezes = 0; //calcular quanto tempo passou 
   char ssid_wifi[ssid.length()+1];
   ssid.toCharArray(ssid_wifi,ssid.length()+1);
   char password_wifi[password.length()];
   password.toCharArray(password_wifi,password.length());
   WiFi.begin(ssid_wifi, password_wifi); // conecta o WiFi recebito pelo server
   //laço responsavel por executar o codigo até se vincular com o WiFi
   while (WiFi.status() != WL_CONNECTED) {
    if(vezes == 29){
      return false;
    }
    delay(500);
    Serial.print(".");
    vezes++;
  }
  Serial.println(WiFi.localIP());
  return true;
}

bool tcp()
{
    int indiceBarra;
    int before , after;
    String ssid_wifi, password_wifi; //informações do pacote sobre o wifi
    String user_mqtt, password_mqtt, ip_mqtt , porta_mqtt;//informações do pacote do mqtt
    if (cl.connected())//Detecta se há clientes conectados no servidor.
    {
        if (cl.available() > 0)//Verifica se o cliente conectado tem dados para serem lidos.
        {
            String req = "";
            while (cl.available() > 0)//Armazena cada Byte (letra/char) na String para formar a mensagem recebida.
            {
                char z = cl.read();
                req += z;
            }
 
            //Mostra a mensagem recebida do cliente no Serial Monitor.
            Serial.print("\nUm cliente enviou uma mensagem");
            Serial.print("\n...IP do cliente: ");
            Serial.print(cl.remoteIP());
            Serial.print("\n...IP do servidor: ");
            Serial.print(WiFi.softAPIP());
            Serial.print("\n...Mensagem do cliente: " + req + "\n");

            //Envia uma resposta para o cliente
            cl.print("\nO servidor recebeu sua mensagem");
            cl.print("\n...Seu IP: ");
            cl.print(cl.remoteIP());
            cl.print("\n...IP do Servidor: ");
            cl.print(WiFi.softAPIP());
            cl.print("\n...Sua mensagem: " + req + "\n");
            
            // pacote = indice/informormações
            indiceBarra = req.indexOf("/"); // pega o indice da barra
            //verifica se é uma palavra
            if(indiceBarra != -1){ //caso seja duas
              int opcao = req.substring(0,indiceBarra).toInt(); 
              if(opcao == 1){
                before = indiceBarra;
                for(int i = 0; i < 2; i++){   
                  after =  req.indexOf("/",before+1);
                  if(after != -1){
                    ssid_wifi = req.substring(before+1,after);
                    before = after;
                  }else{
                    password_wifi = req.substring(before+1);
                  }     
                }
                if(ssid_wifi.length() != 0 && password_wifi.length() != 0){
                  ssid = ssid_wifi;
                  password = password_wifi;
                  Serial.println(ssid);
                  Serial.println(password);
                }
                
              }
              else if(opcao == 2){
                before = indiceBarra;
                for(int i = 0; i < 4; i++){   
                  after =  req.indexOf("/",before+1);
                  if(after != -1){
                    if(i == 0){
                      user_mqtt = req.substring(before+1,after);
                    }
                    else if(i == 1){
                      password_mqtt = req.substring(before+1,after);
                    }
                    else if(i == 2){
                      ip_mqtt = req.substring(before+1,after);
                    }
                    before = after;
                  }else{
                    porta_mqtt = req.substring(before+1);
                  }     
                }
                if(user_mqtt.length() != 0 && password_mqtt.length() != 0 && ip_mqtt.length() != 0 && porta_mqtt.length() != 0){
                  if(porta_mqtt.toInt() != 0){
                    Serial.println(user_mqtt);
                    mqttUser = user_mqtt;
                    Serial.println(password_mqtt);
                    mqttPassword = password_mqtt;
                    Serial.println(ip_mqtt);
                    mqtt_server = ip_mqtt;
                    Serial.println(porta_mqtt.toInt());
                    mqttPort = porta_mqtt.toInt();
                  }
                }
              }    
            }else{
              int teste = req.toInt();
              //opção 1 é pra sair do server
              if(teste == 1){//verificar se é pra sair do server tcp/ip
                return true;
              }
            }
            return false;
        }
    }
    else//Se nao houver cliente conectado,
    {
        cl = sv.available();//Disponabiliza o servidor para o cliente se conectar.
        delay(1);
    }
    return false;
}

//-------------------------------------------------------------------------------------------------------------------

//Bloco do MQTT

//método para pegar o subscribe
void callback(char* topic, byte* payload, unsigned int length) {
  String conteudo= "";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    conteudo.concat(String((char)payload[i]));
  }
  if(conteudo.startsWith("LED")){ // pacote responsavel por ditar o comportamento dos LEDS
    String acaoLED = conteudo.substring(conteudo.indexOf("/") + 1); //qual a acao a ser realizada
    if(acaoLED.startsWith("ASCENDER")){//ascender
      int tempo = acaoLED.substring(acaoLED.indexOf("/") + 1).toInt(); //pega o tempo
      Serial.println(tempo);
    }else if(acaoLED.startsWith("DESLIGAR")){//desligar
      Serial.println("Desligou LED");
    }  
  }else if(conteudo.startsWith("BUZZER")){// pacote responsavel por ditar o comportamento dos BUZZERS
    String acaoBUZZER = conteudo.substring(conteudo.indexOf("/") + 1);//qual a acao a ser realizada
    if(acaoBUZZER.startsWith("TOCAR")){//tocar
      int faixa = acaoBUZZER.substring(acaoBUZZER.indexOf("/") + 1).toInt();//frequencia
      int tempo = acaoBUZZER.substring(acaoBUZZER.lastIndexOf("/") + 1).toInt();//tempo
      Serial.printf("\n Faixa: %d Tempo: %d\n",faixa,tempo);
    }else if(acaoBUZZER.startsWith("DESLIGAR")){
      Serial.println("Desligou BUZZER");
    }
  }
}

//reconectar ao server
void reconnect() {
  char userMqtt[mqttUser.length() + 1];
  mqttUser.toCharArray(userMqtt,mqttUser.length() + 1);
  Serial.printf("Teste: %s \n", userMqtt);
  char passwordMqtt[mqttPassword.length() + 1];
  mqttPassword.toCharArray(passwordMqtt,mqttPassword.length() + 1);
  Serial.printf("Teste: %s \n", passwordMqtt);
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), userMqtt, passwordMqtt)) {
      Serial.println("connected");
      client.publish("outTopic", "hello world");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);//Habilita a comm serial.
  WiFi.mode(WIFI_AP);//Define o WiFi como Acess_Point.
  WiFi.softAP("NodeMCU", "");//Cria a rede de Acess_Point.
  sv.begin();//Inicia o servidor TCP na porta declarada no começo.
}

void loop()
{
   //laço responsavel por executar o codigo até receber todos os dados necessarios para sua execução
   while(servidor == false){ 
    servidor = tcp();//Funçao que gerencia os pacotes e clientes TCP.
   }
   if(wifi == false){
      WiFi.forceSleepBegin(); //desconecta o server TCP
      WiFi.forceSleepWake(); //reabilita a opção do WiFi
      wifi = setup_wifi();
      if(wifi == false){
        servidor = false;
        return;
      }
   }else{
      char serverMqtt[mqtt_server.length() + 1];
      mqtt_server.toCharArray(serverMqtt,mqtt_server.length() + 1);
      client.setServer(serverMqtt, mqttPort);
      client.setCallback(callback);
      if (!client.connected()) {  
        reconnect();
      } 
      client.subscribe("ENTRADA");
      client.loop(); 
   }
}
