# Matriz de LEDs WS2812 Controlada por Teclado Matricial com Raspberry Pi Pico

Este projeto demonstra como controlar uma matriz de LEDs WS2812 (5x5) usando um teclado matricial 4x4 e uma placa Raspberry Pi Pico. O código em C implementa a leitura do teclado, o controle dos LEDs via PIO (Programmable Input/Output) e a reprodução de animações pré-definidas.

## Funcionalidades

*   **Leitura de Teclado Matricial:** Detecta a tecla pressionada no teclado 4x4.
*   **Controle de LEDs WS2812:** Controla individualmente os LEDs da matriz 5x5.
*   **Animações Pré-definidas:** Reproduz animações diferentes com base na tecla pressionada.
*   **Controle de Cor:** Permite ligar todos os LEDs em cores sólidas (azul, vermelho, verde e branco) com diferentes intensidades.
*   **Desligamento dos LEDs:** Desliga todos os LEDs da matriz.
*   **Debounce de Teclas:** Implementa um debounce simples para evitar leituras múltiplas de uma mesma tecla.

## Como Compilar e Executar

1.  **Ambiente de Desenvolvimento:** Configure o ambiente de desenvolvimento para Raspberry Pi Pico (SDK).
2.  **Compilação:** Compile o código C usando o SDK.
3.  **Upload:** Carregue o arquivo `.uf2` gerado para a Raspberry Pi Pico.

## Uso

Após o upload do código, o sistema aguardará a entrada do teclado. As seguintes teclas executam as seguintes ações:

*   **0-7:** Reproduzem animações pré-definidas com diferentes cores e velocidades.
*   **8-9:** Desligam os LEDs e exibem uma mensagem informando que a animação não está definida.
*   **A:** Desliga todos os LEDs.
*   **B:** Liga todos os LEDs em azul (100% de intensidade).
*   **C:** Liga todos os LEDs em vermelho (80% de intensidade).
*   **D:** Liga todos os LEDs em verde (50% de intensidade).
*   **#:** Liga todos os LEDs em branco (20% de intensidade).

## Código

O código principal contém a lógica de leitura do teclado, controle dos LEDs e reprodução das animações. O arquivo `pio_matrix.pio` contém o programa PIO responsável por controlar os LEDs WS2812.

## Observações

*   O código utiliza um debounce simples para o teclado. Para aplicações mais robustas, um debounce mais sofisticado pode ser implementado.
*   As animações são armazenadas em arrays bidimensionais (`animacao0_frames`, `animacao1_frames`, etc.). Cada linha do array representa um frame da animação.
*   A função `matrix_rgb` converte valores RGB normalizados (0.0 - 1.0) para o formato GRB de 24 bits exigido pelos LEDs WS2812.

## Contribuições

Contribuições são bem-vindas! Sinta-se à vontade para abrir issues ou pull requests.
