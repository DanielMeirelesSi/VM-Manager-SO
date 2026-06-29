# VM Manager

Simulador de gerenciamento de memória virtual desenvolvido em C para o Trabalho Prático 2 da disciplina de Sistemas Operacionais.

O programa realiza a tradução de endereços lógicos para endereços físicos usando TLB, tabela de páginas, paginação por demanda, tratamento de page faults e substituição de páginas com LRU aproximado usando aging counters.

## Autor

Daniel Meireles

## Objetivo

O objetivo do projeto é simular os principais passos envolvidos na tradução de endereços em um sistema com memória virtual paginada.

Para cada endereço lógico lido da entrada, o programa:

1. Considera apenas os 16 bits menos significativos.
2. Extrai o número da página e o offset.
3. Consulta o TLB.
4. Consulta a tabela de páginas em caso de TLB miss.
5. Trata page fault quando a página não está residente na memória física.
6. Carrega páginas a partir do arquivo `BACKING_STORE.bin`.
7. Substitui páginas quando a memória física está cheia.
8. Calcula o endereço físico.
9. Lê e imprime o valor armazenado na memória física.
10. Exibe estatísticas finais de execução.

## Configurações do simulador

| Item                                |                              Valor |
| ----------------------------------- | ---------------------------------: |
| Espaço de endereçamento virtual     |                       65.536 bytes |
| Endereço lógico considerado         |                            16 bits |
| Tamanho da página                   |                          256 bytes |
| Tamanho do frame                    |                          256 bytes |
| Número de páginas                   |                                256 |
| Número de frames físicos            |                                128 |
| Tamanho da memória física           |                       32.768 bytes |
| Entradas no TLB                     |                                 16 |
| Política do TLB                     |                               FIFO |
| Política de substituição de páginas | LRU aproximado com Aging Algorithm |

## Estrutura do projeto

```text
vm_manager/
├── Makefile
├── README.md
├── include/
│   ├── config.h
│   ├── memory.h
│   ├── page_table.h
│   ├── statistics.h
│   └── tlb.h
├── src/
│   ├── main.c
│   ├── memory.c
│   ├── page_table.c
│   ├── statistics.c
│   └── tlb.c
├── data/
│   ├── generate_data.py
│   ├── BACKING_STORE.bin
│   ├── addresses_random.txt
│   └── addresses_location.txt
└── report/
```

## Principais módulos

### `main.c`

Contém o fluxo principal do simulador. É responsável por ler os endereços lógicos, extrair página e offset, consultar TLB e tabela de páginas, acionar page fault quando necessário, calcular o endereço físico e imprimir o resultado.

### `statistics.c`

Controla os contadores de endereços traduzidos, page faults e TLB hits. Ao final da execução, calcula e imprime as taxas de page fault e TLB hit.

### `page_table.c`

Implementa a tabela de páginas. Cada entrada armazena o frame correspondente, o bit de validade, o bit de referência e o aging counter usado no LRU aproximado.

### `tlb.c`

Implementa o Translation Lookaside Buffer com 16 entradas e política FIFO. O TLB armazena pares página/frame para acelerar a tradução de endereços.

### `memory.c`

Implementa a memória física, o carregamento de páginas a partir do `BACKING_STORE.bin`, o tratamento de page faults, o controle de frames livres e a substituição de páginas.

## Algoritmo de tradução

O fluxo de tradução usado pelo programa é:

```text
Ler endereço lógico
Aplicar máscara de 16 bits
Extrair número da página
Extrair offset
Consultar TLB

Se houver TLB hit:
    Usar o frame retornado pelo TLB

Se houver TLB miss:
    Consultar a tabela de páginas

    Se a página estiver válida:
        Usar o frame da tabela de páginas

    Se a página não estiver válida:
        Contar page fault
        Carregar a página do BACKING_STORE.bin
        Atualizar memória física
        Atualizar tabela de páginas

    Inserir página/frame no TLB

Marcar página como referenciada
Atualizar aging counters
Calcular endereço físico
Ler valor da memória física
Imprimir resultado
```

## Substituição de páginas

A memória física possui apenas 128 frames, enquanto o espaço virtual possui 256 páginas. Por isso, quando todos os frames estão ocupados, é necessário substituir uma página.

A política usada é LRU aproximado com Aging Algorithm.

Cada página residente possui:

1. Um bit de referência.
2. Um contador de envelhecimento de 8 bits.

A cada acesso, o bit de referência da página é marcado. Em seguida, os aging counters são atualizados:

1. O contador é deslocado uma posição para a direita.
2. O bit de referência é inserido no bit mais significativo.
3. O bit de referência é zerado.

Quando é necessário substituir uma página, o simulador escolhe a página válida com menor aging counter.

## Geração dos arquivos de teste

Entre na pasta `data` e execute o script Python:

```bash
cd data
python3 generate_data.py
cd ..
```

Esse comando gera os arquivos:

```text
BACKING_STORE.bin
addresses_random.txt
addresses_location.txt
```

## Compilação

Na raiz do projeto, execute:

```bash
make
```

Para limpar o executável gerado e recompilar do zero:

```bash
make clean
make
```

## Execução

Para executar com endereços aleatórios:

```bash
./vm < data/addresses_random.txt
```

Para executar com endereços com localidade de referência:

```bash
./vm < data/addresses_location.txt
```

Também é possível salvar a saída em arquivos:

```bash
./vm < data/addresses_random.txt > output_random.txt
./vm < data/addresses_location.txt > output_location.txt
```

## Formato da saída

Para cada endereço traduzido, o programa imprime:

```text
Logical address: <endereco_logico> Physical address: <endereco_fisico> Value: <valor>
```

Ao final da execução, imprime:

```text
Number of Translated Addresses = <total>
Page Faults = <quantidade>
Page Fault Rate = <taxa>
TLB Hits = <quantidade>
TLB Hit Rate = <taxa>
```

## Testes realizados

Foram realizados testes com os dois arquivos gerados pelo script `generate_data.py`.

### Teste com `addresses_random.txt`

Resultado obtido:

```text
Number of Translated Addresses = 10000
Page Faults = 4958
Page Fault Rate = 0.496
TLB Hits = 657
TLB Hit Rate = 0.066
```

### Teste com `addresses_location.txt`

Resultado obtido:

```text
Number of Translated Addresses = 10000
Page Faults = 1164
Page Fault Rate = 0.116
TLB Hits = 7974
TLB Hit Rate = 0.797
```

O arquivo com localidade de referência apresentou menor taxa de page faults e maior taxa de TLB hits, o que é coerente com acessos repetidos ou próximos na memória.

## Comandos de validação usados

Compilação limpa:

```bash
make clean && make
```

Execução dos testes:

```bash
./vm < data/addresses_random.txt > output_random.txt
./vm < data/addresses_location.txt > output_location.txt
```

Conferência das estatísticas:

```bash
tail -n 10 output_random.txt
tail -n 10 output_location.txt
```

Conferência da quantidade de linhas:

```bash
wc -l output_random.txt output_location.txt
```

Conferência de endereços físicos dentro da memória física:

```bash
awk '/Physical address:/ { if ($6 < 0 || $6 > 32767) { print "Endereco fisico invalido:", $0; exit 1 } } END { print "Conferencia de enderecos fisicos finalizada." }' output_random.txt
```

```bash
awk '/Physical address:/ { if ($6 < 0 || $6 > 32767) { print "Endereco fisico invalido:", $0; exit 1 } } END { print "Conferencia de enderecos fisicos finalizada." }' output_location.txt
```

## Limpeza de arquivos gerados

Para remover o executável:

```bash
make clean
```

Os arquivos `output_random.txt`, `output_location.txt` e o executável `vm` são arquivos gerados localmente e não precisam ser versionados.
```
