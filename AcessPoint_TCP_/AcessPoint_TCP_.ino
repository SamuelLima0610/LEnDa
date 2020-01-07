#include <ESP8266WiFi.h>
#include <WiFiServer.h>

WiFiServer sv(555);//Cria o objeto servidor na porta 555
WiFiClient cl;//Cria o objeto cliente.
bool informado = false; //variavel controla o laco de receber informacoes pela tcp
String nomeWifi; // variavel responsavel por armazenar o usuario do WiFi
String senhaWifi;// variavel responsavel por armazenar a senha do WiFi
const char* ssid = "";
const char* password = "";

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
   while(informado == false){ 
    informado = tcp();//Funçao que gerencia os pacotes e clientes TCP.
   }
   WiFi.disconnect(); //desconecta o server TCP
   WiFi.begin(ssid, password); // conecta o WiFi recebito pelo server
   //laço responsavel por executar o codigo até se vincular com o WiFi
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}

bool tcp()
{
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
            
            //trata a informacao("usuario/senha")
            int indiceBarra = req.indexOf("/");
            String conteudoAntesDaBarra = req.substring(0,indiceBarra); 
            String conteudoDepoisDaBarra = req.substring(indiceBarra+1);      
            if(conteudoAntesDaBarra.length() != 0 && conteudoDepoisDaBarra.length() != 0){
              nomeWifi = conteudoAntesDaBarra;
              senhaWifi = conteudoDepoisDaBarra;
              return true;
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
