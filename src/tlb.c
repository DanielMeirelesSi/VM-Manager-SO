#include "tlb.h"
#include "config.h"

static tlb_entry_t tlb[TLB_SIZE];

/*
 * Guarda a ordem de inserção de cada entrada válida.
 * Quanto menor o valor, mais antiga é a entrada.
 */
static int fifo_order[TLB_SIZE];

/*
 * Próximo valor de ordem a ser usado em uma nova inserção.
 */
static int next_fifo_order = 0;

static int is_valid_page_number(int page)
{
    return page >= 0 && page < PAGE_TABLE_SIZE;
}

static int find_page_index(int page)
{
    if (!is_valid_page_number(page)) {
        return -1;
    }

    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid && tlb[i].page == page) {
            return i;
        }
    }

    return -1;
}

static int find_invalid_index(void)
{
    for (int i = 0; i < TLB_SIZE; i++) {
        if (!tlb[i].valid) {
            return i;
        }
    }

    return -1;
}

static int find_fifo_victim_index(void)
{
    int victim_index = -1;
    int oldest_order = 0;

    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].valid) {
            if (victim_index == -1 || fifo_order[i] < oldest_order) {
                victim_index = i;
                oldest_order = fifo_order[i];
            }
        }
    }

    return victim_index;
}

static void set_tlb_entry(int index, int page, int frame)
{
    tlb[index].page = page;
    tlb[index].frame = frame;
    tlb[index].valid = 1;
    fifo_order[index] = next_fifo_order;
    next_fifo_order++;
}

void tlb_init(void)
{
    for (int i = 0; i < TLB_SIZE; i++) {
        tlb[i].page = -1;
        tlb[i].frame = -1;
        tlb[i].valid = 0;
        fifo_order[i] = 0;
    }

    next_fifo_order = 0;
}

int tlb_lookup(int page)
{
    int index = find_page_index(page);

    if (index == -1) {
        return -1;
    }

    return tlb[index].frame;
}

void tlb_insert(int page, int frame)
{
    if (!is_valid_page_number(page)) {
        return;
    }

    int index = find_page_index(page);

    if (index != -1) {
        tlb[index].frame = frame;
        return;
    }

    index = find_invalid_index();

    if (index == -1) {
        index = find_fifo_victim_index();
    }

    if (index == -1) {
        return;
    }

    set_tlb_entry(index, page, frame);
}

void tlb_remove(int page)
{
    int index = find_page_index(page);

    if (index == -1) {
        return;
    }

    tlb[index].page = -1;
    tlb[index].frame = -1;
    tlb[index].valid = 0;
    fifo_order[index] = 0;
}

void tlb_clear(void)
{
    tlb_init();
}