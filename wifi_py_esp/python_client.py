import socket
import speech_recognition as sr
import difflib
import re
import time

class ESP32Controller:
    def __init__(self, esp32_ip, esp32_port):
        self.esp32_ip = esp32_ip
        self.esp32_port = esp32_port
        self.sock = None
        self.connected = False
    
    def conectar(self):
        """Tenta conectar ao ESP32"""
        try:
            if self.sock:
                try:
                    self.sock.close()
                except:
                    pass
            
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(3)
            self.sock.connect((self.esp32_ip, self.esp32_port))
            self.sock.settimeout(1)
            
            self.connected = True
            print(f"[OK] Conectado ao ESP32 ({self.esp32_ip})")
            return True
            
        except Exception as e:
            self.connected = False
            print(f"[ERRO] Falha na conexao: {e}")
            return False
    
    def enviar_comando(self, comando):
        """Envia comando e retorna True se bem sucedido"""
        if not self.connected:
            return False
            
        try:
            self.sock.send((comando + "\n").encode('utf-8'))
            print(f"[ENVIADO] Comando '{comando}'")
            
            # Tenta ler resposta
            try:
                resposta = self.sock.recv(1024).decode('utf-8').strip()
                if resposta:
                    print(f"  Resposta ESP32: {resposta}")
            except socket.timeout:
                pass
                
            return True
            
        except Exception as e:
            print(f"[ERRO] Comunicacao: {e}")
            self.connected = False
            return False
    
    def fechar(self):
        if self.sock:
            self.sock.close()
            self.sock = None
            self.connected = False

def interpretar_comando(texto):
    """
    Interpreta o comando de voz e retorna '1', '0' ou None
    """
    if not texto:
        return None
    
    texto = texto.lower().strip()
    print(f"[DEBUG] Texto processado: '{texto}'")
    
    palavras_desligar = ["desligar", "desliga", "desligue", "desligado", "desliguei", "desligamos", "desligaram"]
    
    # Verificacao 1: Correspondencia exata de palavras
    palavras = texto.split()
    for palavra in palavras:
        if palavra == "amarelo":
            print(f"  -> Comando LIGAR (palavra: '{palavra}')")
            return "amarelo"
        if palavra == "vermelho":
            print(f"  -> Comando LIGAR (palavra: '{palavra}')")
            return "vermelho"
        if palavra == "azul":
            print(f"  -> Comando LIGAR (palavra: '{palavra}')")
            return "azul"
    
    
    for palavra_alvo in palavras_desligar:
        similaridade = difflib.SequenceMatcher(None, texto, palavra_alvo).ratio()
        if similaridade > 0.7:
            print(f"  -> Comando DESLIGAR (similaridade {similaridade:.2f} com '{palavra_alvo}')")
            return "desligar"
    
    print("  -> Comando nao reconhecido")
    return None

def ouvir_comando():
    """Captura audio do microfone e reconhece comando"""
    r = sr.Recognizer()
    
    with sr.Microphone() as source:
        print("\n" + "="*40)
        print("Ajustando para ruido ambiente...")
        r.adjust_for_ambient_noise(source, duration=0.5)
        
        print(">>> Pode falar (ou diga 'encerrar'):")
        
        try:
            audio = r.listen(source, timeout=5, phrase_time_limit=3)
            texto = r.recognize_google(audio, language="pt-BR")
            print(f"Reconhecido: '{texto}'")
            return texto.lower().strip()
            
        except sr.WaitTimeoutError:
            print("Nada detectado (timeout)")
            return None
        except sr.UnknownValueError:
            print("Nao entendi o que disse")
            return None
        except Exception as e:
            print(f"Erro no reconhecimento: {e}")
            return None

def main():
    # Configuracoes
    ESP32_IP = "10.0.0.53"  # Substitua pelo IP do seu ESP32
    ESP32_PORT = 8888
    
    print("\n" + "="*50)
    print("CONTROLE DE LED POR VOZ (WiFi)")
    print("="*50)
    print(f"ESP32: {ESP32_IP}:{ESP32_PORT}")
    print("Comandos: 'ligar', 'desligar', 'sair'")
    print("="*50 + "\n")
    
    # Conecta ao ESP32
    esp32 = ESP32Controller(ESP32_IP, ESP32_PORT)
    
    tentativas = 0
    while not esp32.conectar() and tentativas < 3:
        print(f"Tentativa {tentativas + 1} de 3. Tentando novamente em 3 segundos...")
        time.sleep(3)
        tentativas += 1
    
    if not esp32.connected:
        print("[ERRO] Nao foi possivel conectar ao ESP32. Verifique:")
        print("  - Se o ESP32 esta ligado")
        print("  - Se o IP esta correto")
        print("  - Se ambos estao na mesma rede WiFi")
        return
    
    try:
        while True:
            # Verifica conexao
            if not esp32.connected:
                print("\n[Tentando reconectar ao ESP32...]")
                if not esp32.conectar():
                    print("[Aguardando 5 segundos...]")
                    time.sleep(5)
                    continue
            
            # Captura comando de voz
            comando = ouvir_comando()
            
            if comando is None:
                continue
            
            # Verifica comando de sair
            if "sair" in comando or "encerrar" in comando:
                print("[Encerrando programa...]")
                break
            
            # Interpreta o comando
            msg = interpretar_comando(comando)
            
            if msg:
                esp32.enviar_comando(msg)
            else:
                print("[?] Comando nao reconhecido. Tente 'ligar' ou 'desligar'")
                    
    except KeyboardInterrupt:
        print("\n[Programa interrompido pelo usuario]")
    
    finally:
        esp32.fechar()
        print("[Conexao fechada]")

if __name__ == "__main__":
    main()
