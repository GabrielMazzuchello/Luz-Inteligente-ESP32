#include <WiFi.h>
#include <ThingESP.h>

// Definindo os pinos para as luzes e sensor magnético
int LED_COZINHA = 23;
int LED_SALA = 22;
int LED_QUARTO = 19;
int SENSOR_PORTA = 12;  // Pino do sensor magnético MC-38
bool porta_aberta = false;

// Definindo os pinos dos botões
int BOTAO_COZINHA = 13;
int BOTAO_SALA = 14;
int BOTAO_QUARTO = 25;

// Estado das luzes
bool luzCozinhaLigada = false;
bool luzSalaLigada = false;
bool luzQuartoLigada = false;

// Variáveis para controle de debounce, separadas por botão
unsigned long debounceDelay = 200; // Tempo de debounce em milissegundos
unsigned long lastButtonPressCozinha = 0;
unsigned long lastButtonPressSala = 0;
unsigned long lastButtonPressQuarto = 0;

unsigned long ultimoTempo = 0;  // Último tempo registrado
const unsigned long intervalo = 200;  // Intervalo de 200 ms para debounce

// Configuração do ThingESP para controle via celular
ThingESP32 thing("gabrieldalmolin", "luzinteligente", "123456");

void setup() {
    Serial.begin(115200);

    // Definindo os pinos das luzes como saída
    pinMode(LED_COZINHA, OUTPUT);
    pinMode(LED_SALA, OUTPUT);
    pinMode(LED_QUARTO, OUTPUT);

    // Inicia as luzes desligadas (relés em HIGH)
    digitalWrite(LED_COZINHA, HIGH);
    digitalWrite(LED_SALA, HIGH);
    digitalWrite(LED_QUARTO, HIGH);

    // Definindo os pinos dos botões e do sensor como entrada
    pinMode(BOTAO_COZINHA, INPUT_PULLUP);
    pinMode(BOTAO_SALA, INPUT_PULLUP);
    pinMode(BOTAO_QUARTO, INPUT_PULLUP);
    pinMode(SENSOR_PORTA, INPUT);

    // Conectando o ThingESP à rede WiFi
    thing.SetWiFi("Redmi Note 12", "12345678");
    thing.initDevice(); 
}

// Função para controlar as luzes pelos botões
void verificarBotao(int botao, int led, String nomeLuz, bool &luzLigada, unsigned long &lastPressTime) {
    if (digitalRead(botao) == LOW) {  // Botão pressionado (nível LOW)
        unsigned long currentMillis = millis();
        if (currentMillis - lastPressTime > debounceDelay) {
            lastPressTime = currentMillis;  // Atualiza o tempo da última pressão
            if (!luzLigada) {  // Se a luz está desligada, ligamos
                digitalWrite(led, LOW);  // Liga o LED
                luzLigada = true;  // Atualiza o estado da variável
                Serial.println("Luz ligada " + nomeLuz);
            } else {
                digitalWrite(led, HIGH);  // Desliga o LED
                luzLigada = false;  // Atualiza o estado da variável
                Serial.println("Luz desligada " + nomeLuz);
            }
        }
    }
}

// Função para processar comandos via celular (ThingESP)
String HandleResponse(String query) {
    // Menu de opções
    String menu = "\n\nMenu:\n1 - Acender luz da cozinha\n2 - Acender luz da sala\n3 - Acender luz do quarto\n-1 - Apagar luz da cozinha\n-2 - Apagar luz da sala\n-3 - Apagar luz do quarto\n0 - Apagar todas as luzes";

    // Processamento dos comandos
    if (query == "1") {
        if (!luzCozinhaLigada) {
            digitalWrite(LED_COZINHA, LOW);
            luzCozinhaLigada = true;
            return "A luz da cozinha foi ligada." + menu;
        }
    } else if (query == "2") {
        if (!luzSalaLigada) {
            digitalWrite(LED_SALA, LOW);
            luzSalaLigada = true;
            return "A luz da sala foi ligada." + menu;
        }
    } else if (query == "3") {
        if (!luzQuartoLigada) {
            digitalWrite(LED_QUARTO, LOW);
            luzQuartoLigada = true;
            return "A luz do quarto foi ligada." + menu;
        }
    } else if (query == "-1") {
        if (luzCozinhaLigada) {
            digitalWrite(LED_COZINHA, HIGH);
            luzCozinhaLigada = false;
            return "A luz da cozinha foi desligada." + menu;
        }
    } else if (query == "-2") {
        if (luzSalaLigada) {
            digitalWrite(LED_SALA, HIGH);
            luzSalaLigada = false;
            return "A luz da sala foi desligada." + menu;
        }
    } else if (query == "-3") {
        if (luzQuartoLigada) {
            digitalWrite(LED_QUARTO, HIGH);
            luzQuartoLigada = false;
            return "A luz do quarto foi desligada." + menu;
        }
    } else if (query == "0") {
        digitalWrite(LED_COZINHA, HIGH);
        digitalWrite(LED_SALA, HIGH);
        digitalWrite(LED_QUARTO, HIGH);
        luzCozinhaLigada = false;
        luzSalaLigada = false;
        luzQuartoLigada = false;
        return "Todas as luzes foram desligadas." + menu;
    } else {
        // Comando inválido
        return "Comando inválido." + menu;
    }

    return menu;  // Garante que o menu seja exibido se o comando for válido
}

void loop() {
    // Controle pelos botões
    verificarBotao(BOTAO_COZINHA, LED_COZINHA, "cozinha", luzCozinhaLigada, lastButtonPressCozinha);
    verificarBotao(BOTAO_SALA, LED_SALA, "sala", luzSalaLigada, lastButtonPressSala);
    verificarBotao(BOTAO_QUARTO, LED_QUARTO, "quarto", luzQuartoLigada, lastButtonPressQuarto);

    // Verificar o estado da porta
    int estadoPorta = digitalRead(SENSOR_PORTA);

    unsigned long tempoAtual = millis();
    // Porta aberta detectada
    if (estadoPorta == HIGH && !porta_aberta && (tempoAtual - ultimoTempo >= intervalo)) {
        porta_aberta = true;
        ultimoTempo = tempoAtual;  // Atualiza o último tempo
        digitalWrite(LED_COZINHA, LOW);  // Liga a luz da cozinha
        luzCozinhaLigada = true;  // Atualiza o estado da luz
        Serial.println("Luz da cozinha ligada pela porta");
    }
    // Porta fechada detectada
    else if (estadoPorta == LOW && porta_aberta && (tempoAtual - ultimoTempo >= intervalo)) {
        porta_aberta = false;
        ultimoTempo = tempoAtual;  // Atualiza o último tempo
        Serial.println("Porta fechada");
    }

    // Processar comandos via celular
    thing.Handle();
}
