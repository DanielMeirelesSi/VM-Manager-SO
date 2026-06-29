#include <stdio.h>
#include <stdlib.h>

#include "memory.h"
#include "config.h"
#include "page_table.h"
#include "tlb.h"

static signed char physical_memory[NUM_FRAMES][FRAME_SIZE];

/*
 * Indica qual página está carregada em cada quadro.
 * Valor -1 indica quadro livre.
 */
static int frame_to_page[NUM_FRAMES];

static FILE *backing = NULL;

static int is_valid_page_number(int page)
{
    return page >= 0 && page < PAGE_TABLE_SIZE;
}

static int is_valid_frame_number(int frame)
{
    return frame >= 0 && frame < NUM_FRAMES;
}

static int is_valid_offset(int offset)
{
    return offset >= 0 && offset < FRAME_SIZE;
}

void memory_init(FILE *backing_store)
{
    backing = backing_store;

    for (int i = 0; i < NUM_FRAMES; i++) {
        frame_to_page[i] = -1;

        for (int j = 0; j < FRAME_SIZE; j++) {
            physical_memory[i][j] = 0;
        }
    }
}

static int find_free_frame(void)
{
    for (int i = 0; i < NUM_FRAMES; i++) {
        if (frame_to_page[i] == -1) {
            return i;
        }
    }

    return -1;
}

int handle_page_fault(int page)
{
    if (!is_valid_page_number(page)) {
        fprintf(stderr, "Erro: pagina invalida: %d\n", page);
        exit(1);
    }

    if (backing == NULL) {
        fprintf(stderr, "Erro interno: BACKING_STORE nao inicializado.\n");
        exit(1);
    }

    int frame = find_free_frame();

    if (frame == -1) {
        int victim_page = select_victim_page();

        if (!is_valid_page_number(victim_page)) {
            fprintf(stderr, "Erro: nao foi possivel selecionar pagina vitima.\n");
            exit(1);
        }

        frame = page_table_get_frame(victim_page);

        if (!is_valid_frame_number(frame)) {
            fprintf(stderr, "Erro: frame invalido para pagina vitima %d.\n", victim_page);
            exit(1);
        }

        page_table_invalidate(victim_page);
        tlb_remove(victim_page);
        frame_to_page[frame] = -1;
    }

    long backing_position = (long) page * PAGE_SIZE;

    if (fseek(backing, backing_position, SEEK_SET) != 0) {
        fprintf(stderr, "Erro: nao foi possivel posicionar BACKING_STORE na pagina %d.\n", page);
        exit(1);
    }

    size_t bytes_read = fread(physical_memory[frame], sizeof(signed char), PAGE_SIZE, backing);

    if (bytes_read != PAGE_SIZE) {
        fprintf(stderr, "Erro: nao foi possivel ler a pagina %d do BACKING_STORE.\n", page);
        exit(1);
    }

    frame_to_page[frame] = page;
    page_table_update(page, frame);

    return frame;
}

int select_victim_page(void)
{
    int victim_page = -1;
    unsigned char lowest_counter = 0;

    for (int page = 0; page < PAGE_TABLE_SIZE; page++) {
        if (page_table_is_valid(page)) {
            unsigned char current_counter = page_table_get_aging_counter(page);

            if (victim_page == -1 || current_counter < lowest_counter) {
                victim_page = page;
                lowest_counter = current_counter;
            }
        }
    }

    return victim_page;
}

signed char read_memory(int frame, int offset)
{
    if (!is_valid_frame_number(frame)) {
        fprintf(stderr, "Erro: frame invalido: %d\n", frame);
        exit(1);
    }

    if (!is_valid_offset(offset)) {
        fprintf(stderr, "Erro: offset invalido: %d\n", offset);
        exit(1);
    }

    return physical_memory[frame][offset];
}

int get_page_loaded_in_frame(int frame)
{
    if (frame < 0 || frame >= NUM_FRAMES) {
        return -1;
    }

    return frame_to_page[frame];
}