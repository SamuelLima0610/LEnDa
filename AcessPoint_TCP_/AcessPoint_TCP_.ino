#include <ESP8266WiFi.h>
#include <WiFiServer.h>

WiFiServer sv(555);//Cria o objeto servidor na porta 555
WiFiClient cl;//Cria o objeto cliente.
bool servidor = false; //variavel controla o laco de receber informacoes pela tcp
bool wifi = false; //variavel controla o estado do WiFi
String mqtt_server = "";
String ssid = "";// variavel responsavel por armazenar o usuario do WiFi
String password = "";// variavel responsavel por armazenar a senha do WiFi

//função que configura o WiFi a ser usado para o protocolo MQTT(true - conectou false não-conectou)
//tempo limite para conectar 30s
bool setup_wifi(){
   int vezes = 0; //calcular quanto tempo passou 
   char ssid_wifi[ssid.length()];
   ssid.toCharArray(ssid_wifi,ssid.length());
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
  return true;
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
            //verifica se é uma palavra
            if(indiceBarra != -1){ //caso seja duas
              String conteudoAntesDaBarra = req.substring(0,indiceBarra); 
              String conteudoDepoisDaBarra = req.substring(indiceBarra+1);      
              if(conteudoAntesDaBarra.length() != 0 && conteudoDepoisDaBarra.length() != 0){
                //verifica se é informação do ip
                if(conteudoAntesDaBarra.equals("ip")){
                  mqtt_server = conteudoDepoisDaBarra;
                }
                else{ //senão for ip, só pode ser a configuração do WiFi
                  ssid = conteudoAntesDaBarra;
                  password = conteudoDepoisDaBarra;
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
   WiFi.disconnect(); //desconecta o server TCP
   if(wifi == false){
      wifi = setup_wifi();
      if(wifi == false){
        servidor = false;
        return;
      }
   }
}
