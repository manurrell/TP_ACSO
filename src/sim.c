#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "shell.h"
#include <stdint.h>

void execute_hlt();
void execute_adds_immediate(uint32_t instruction);
void execute_adds_register(uint32_t instruction);
void execute_subs_immediate(uint32_t instruction);
void execute_subs_register(uint32_t instruction);
void execute_cmp_immediate(uint32_t instruction);
void execute_cmp_register(uint32_t instruction);
void execute_ands(uint32_t instruction);
void execute_eor(uint32_t instruction);
void execute_orr(uint32_t instruction);
void execute_b(uint32_t instruction);
void execute_br(uint32_t instruction);
void execute_beq(uint32_t instruction);
void execute_bne(uint32_t instruction);
void execute_bgt(uint32_t instruction);
void execute_blt(uint32_t instruction);
void execute_bge(uint32_t instruction);
void execute_ble(uint32_t instruction);
void execute_lsl(uint32_t instruction);
void execute_lsr(uint32_t instruction);
void execute_stur(uint32_t instruction);
void execute_sturb(uint32_t instruction);
void execute_sturh(uint32_t instruction);
void execute_ldur(uint32_t instruction);
void execute_ldurb(uint32_t instruction);
void execute_ldurh(uint32_t instruction);
void execute_movz(uint32_t instruction);
void execute_add_immediate(uint32_t instruction);
void execute_add_register(uint32_t instruction);
void execute_mul(uint32_t instruction);
void execute_cbz(uint32_t instruction);
void execute_cbnz(uint32_t instruction);

void process_instruction() {
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    uint32_t opcode = (instruction >> 24) & 0xFF;

    uint8_t rd = instruction & 0x1F;
    uint8_t cond = instruction & 0xF;
    uint8_t bit22 = (instruction >> 22) & 0x1;
    uint8_t imms = (instruction >> 10) & 0x3F;

    printf("Instrucción: 0x%08X, Opcode: 0x%03X\n", instruction, opcode);

    switch (opcode) {
        case 0xB1:
            execute_adds_immediate(instruction);
            break;
        case 0xAB:
            execute_adds_register(instruction);
            break;
        case 0xF1:
            execute_subs_immediate(instruction);
            break;
        case 0xEB:
            if (rd == 31) {
                execute_cmp_register(instruction);
                break;
            } else {
                execute_subs_register(instruction);
                break;
            }
            break;
        case 0xEA:
            execute_ands(instruction);
            break;
        case 0xCA:
            execute_eor(instruction);
            break;
        case 0xD2:
            execute_movz(instruction);
            break;
        case 0x54:
            if (cond == 0x0) { 
                execute_beq(instruction);
            } else if (cond == 0x1) {
                execute_bne(instruction);
            } else if (cond == 0xC) {
                execute_bgt(instruction); 
            } else if (cond == 0xA) {
                execute_bge(instruction); 
            } else if (cond == 0xD) {
                execute_ble(instruction); 
            } else if (cond == 0xB) {
                execute_blt(instruction);
            }
            break;
        case 0xD3:
            if (imms == 0x3F) { 
                execute_lsr(instruction);
            } else {
                execute_lsl(instruction);
            }
            break;
        case 0xF8:
            if (bit22 == 0) {
                execute_stur(instruction);
            } else {
                execute_ldur(instruction);
            }
            break;
        case 0x38:
            if (bit22 == 0) {
                execute_sturb(instruction);
            } else {
                execute_ldurb(instruction);
            }
            break;
        case 0x9B:
            execute_mul(instruction);
            break;
        case 0xAA:
            execute_orr(instruction);
            break;
        case 0x14:
            execute_b(instruction);
            break;
        case 0x78:
            if (bit22 == 1) {
                execute_ldurh(instruction);
            } else {
                execute_sturh(instruction);
            }
            break;
        case 0xD6: 
            execute_br(instruction);
            break;
        case 0xB4:
            execute_cbz(instruction);
            break;
        case 0xB5:
            execute_cbnz(instruction);
            break;
        case 0x91:
            execute_add_immediate(instruction);
            break;
        case 0x8B:
            execute_add_register(instruction);
            break;
        case 0xD4:
            execute_hlt();
            break;
        default:
            printf("Instrucción no implementada: 0x%08X\n", instruction);
            RUN_BIT = 0;
            break;
    }
}

void execute_hlt() {
    RUN_BIT = 0;

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_adds_immediate(uint32_t instruction) {
    uint8_t rd = instruction & 0x1F;
    uint8_t rn = (instruction >> 5) & 0x1F;
    uint16_t imm12 = (instruction >> 10) & 0xFFF;
    uint8_t shift = (instruction >> 22) & 1;
    uint64_t imm_value;

    if (shift == 1) {
        imm_value = imm12 << 12;
    } else {
        imm_value = imm12;
    }

    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] + imm_value;

    NEXT_STATE.FLAG_N = (NEXT_STATE.REGS[rd] < 0);
    NEXT_STATE.FLAG_Z = (NEXT_STATE.REGS[rd] == 0);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_adds_register(uint32_t instruction) {
    uint8_t rd = instruction & 0x1F;
    uint8_t rn = (instruction >> 5) & 0x1F;
    uint8_t rm = (instruction >> 16) & 0x1F;

    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] + CURRENT_STATE.REGS[rm];

    NEXT_STATE.FLAG_N = (NEXT_STATE.REGS[rd] < 0);
    NEXT_STATE.FLAG_Z = (NEXT_STATE.REGS[rd] == 0);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_subs_immediate(uint32_t instruction) {
    uint8_t rd = instruction & 0x1F;
    uint8_t rn = (instruction >> 5) & 0x1F;
    uint16_t imm12 = (instruction >> 10) & 0xFFF;
    uint8_t shift = (instruction >> 22) & 1;
    uint64_t imm_value;

    if (shift == 1) {
        imm_value = imm12 << 12;
    } else {
        imm_value = imm12;
    }

    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] - imm_value;

    NEXT_STATE.FLAG_N = (NEXT_STATE.REGS[rd] < 0);
    NEXT_STATE.FLAG_Z = (NEXT_STATE.REGS[rd] == 0);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_subs_register(uint32_t instruction) {
    uint8_t rd = instruction & 0x1F;
    uint8_t rn = (instruction >> 5) & 0x1F;
    uint8_t rm = (instruction >> 16) & 0x1F;

    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] - CURRENT_STATE.REGS[rm];

    NEXT_STATE.FLAG_N = (NEXT_STATE.REGS[rd] < 0);
    NEXT_STATE.FLAG_Z = (NEXT_STATE.REGS[rd] == 0);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_cmp_immediate(uint32_t instruction) {
    uint8_t rn = (instruction >> 5) & 0x1F;
    uint16_t imm12 = (instruction >> 10) & 0xFFF;
    uint8_t shift = (instruction >> 22) & 1;
    uint64_t imm_value;
    int64_t rn_value;

    if (shift == 1) {
        imm_value = imm12 << 12;
    } else {
        imm_value = imm12;
    }

    if (rn == 31) {
        rn_value = 0;
    } else {
        rn_value = CURRENT_STATE.REGS[rn];
    }

    int64_t result = rn_value - imm_value;

    NEXT_STATE.FLAG_N = (result < 0);
    NEXT_STATE.FLAG_Z = (result == 0);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_cmp_register(uint32_t instruction) {
    uint8_t rn = (instruction >> 5) & 0x1F;
    uint8_t rm = (instruction >> 16) & 0x1F;
    int64_t rn_value;
    int64_t rm_value;

    if (rn == 31) {
        rn_value = 0;
    } else {
        rn_value = CURRENT_STATE.REGS[rn];
    }

    if (rm == 31) {
        rm_value = 0;
    } else {
        rm_value = CURRENT_STATE.REGS[rm];
    }

    int64_t result = rn_value - rm_value;

    NEXT_STATE.FLAG_N = (result < 0);
    NEXT_STATE.FLAG_Z = (result == 0);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_ands(uint32_t instruction) {
    uint8_t rd = instruction & 0x1F;
    uint8_t rn = (instruction >> 5) & 0x1F;
    uint8_t rm = (instruction >> 16) & 0x1F;

    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] & CURRENT_STATE.REGS[rm];

    NEXT_STATE.FLAG_N = (NEXT_STATE.REGS[rd] < 0);
    NEXT_STATE.FLAG_Z = (NEXT_STATE.REGS[rd] == 0);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_eor(uint32_t instruction) {
    uint8_t rd = instruction & 0x1F;
    uint8_t rn = (instruction >> 5) & 0x1F;
    uint8_t rm = (instruction >> 16) & 0x1F;

    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] ^ CURRENT_STATE.REGS[rm];

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_orr(uint32_t instruction) {
    uint8_t rd = instruction & 0x1F;
    uint8_t rn = (instruction >> 5) & 0x1F;
    uint8_t rm = (instruction >> 16) & 0x1F;

    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] | CURRENT_STATE.REGS[rm];

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_movz(uint32_t instruction) {
    uint8_t rd = instruction & 0x1F;
    uint16_t imm16 = (instruction >> 5) & 0xFFFF;

    NEXT_STATE.REGS[rd] = imm16;

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

// B

void execute_b(uint32_t instruction) {
    int32_t offset = (instruction & 0x03FFFFFF);

    if (offset & (1 << 25)) {
        offset |= 0xFC000000;
    }

    offset = offset << 2;

    NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    CURRENT_STATE = NEXT_STATE;
}

void execute_br(uint32_t instruction) {
    int32_t extracted_bits = (instruction >> 5) & (0x1F);

    NEXT_STATE.PC = CURRENT_STATE.REGS[extracted_bits];
    
    CURRENT_STATE = NEXT_STATE;
}

void execute_beq(uint32_t instruction) {
    int32_t offset = (instruction >> 5) & 0x7FFFF;

    if (offset & (1 << 18)) {
        offset |= 0xFFF80000;
    }

    offset = offset << 2; 

    if (CURRENT_STATE.FLAG_Z) {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    } else { 
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
    CURRENT_STATE = NEXT_STATE;
}

void execute_bgt(uint32_t instruction) {
    int32_t offset = (instruction >> 5) & 0x7FFFF;

    if (offset & (1 << 18)) {
        offset |= 0xFFF80000;
    }

    offset = offset << 2;

    if (!CURRENT_STATE.FLAG_N && !CURRENT_STATE.FLAG_Z) {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    } else { 
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
    CURRENT_STATE = NEXT_STATE;
}

void execute_bge(uint32_t instruction) {
    int32_t offset = (instruction >> 5) & 0x7FFFF;

    if (offset & (1 << 18)) {
        offset |= 0xFFF80000;
    }

    offset = offset << 2;

    if (!CURRENT_STATE.FLAG_N) {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    } else { 
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
    CURRENT_STATE = NEXT_STATE;
}

void execute_ble(uint32_t instruction) {
    int32_t offset = (instruction >> 5) & 0x7FFFF;

    if (offset & (1 << 18)) {
        offset |= 0xFFF80000;
    }

    offset = offset << 2;

    if (CURRENT_STATE.FLAG_N || CURRENT_STATE.FLAG_Z) {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    } else { 
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
    CURRENT_STATE = NEXT_STATE;
}

void execute_bne(uint32_t instruction) {
    int32_t offset = (instruction >> 5) & 0x7FFFF;

    if (offset & (1 << 18)) {
        offset |= 0xFFF80000;
    }

    offset = offset << 2; 

    if (!CURRENT_STATE.FLAG_Z) {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    } else { 
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
    CURRENT_STATE = NEXT_STATE;
}

void execute_blt(uint32_t instruction) {
    int32_t offset = (instruction >> 5) & 0x7FFFF;

    if (offset & (1 << 18)) {
        offset |= 0xFFF80000;
    }

    offset = offset << 2;

    if (CURRENT_STATE.FLAG_N) {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    } else { 
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
    CURRENT_STATE = NEXT_STATE;
}

void execute_lsl(uint32_t instruction) {
    uint8_t rd = instruction & 0x1F;
    uint8_t rn = (instruction >> 5) & 0x1F;
    uint8_t imm6 = 63 - (instruction >> 10) & 0x3F;

    printf("LSL -> rd: %d, rn: %d, imm6: %d (0x%02X)\n", rd, rn, imm6, imm6);

    NEXT_STATE.REGS[rd] = (uint32_t)CURRENT_STATE.REGS[rn] << imm6;

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_lsr(uint32_t instruction) {
    uint8_t rd = instruction & 0x1F;
    uint8_t rn = (instruction >> 5) & 0x1F;
    uint8_t imm6 = (instruction >> 16) & 0x3F;


    NEXT_STATE.REGS[rd] = (uint32_t)CURRENT_STATE.REGS[rn] >> imm6;

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_stur(uint32_t instruction) {
    uint8_t rt = instruction & 0x1F;
    uint8_t rn = (instruction >> 5) & 0x1F;
    int64_t offset = ((instruction >> 12) & 0x2FF);

    if (offset & (1 << 8)) {
        offset |= 0xFFFFFFFFFFFFFE00;
    }

    uint64_t address = CURRENT_STATE.REGS[rn] + offset;
    uint64_t value = CURRENT_STATE.REGS[rt];

    if (address < 0x10000000 || address >= 0x10100000) {
        printf("STUR: Dirección 0x%016lX fuera de rango\n", address);
        return;
    }

    mem_write_32(address, value & 0xFFFFFFFF);
    mem_write_32(address + 4, (value >> 32) & 0xFFFFFFFF);


    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_sturb(uint32_t instruction) {
    uint8_t rt = instruction & 0x1F;
    uint8_t rn = (instruction >> 5) & 0x1F;
    int64_t offset = ((instruction >> 12) & 0x2FF);

    if (offset & (1 << 8)) {
        offset |= 0xFFFFFFFFFFFFFE00;
    }

    uint64_t address = CURRENT_STATE.REGS[rn] + offset;
    uint8_t byte_val = CURRENT_STATE.REGS[rt] & 0xFF;

    if (address < 0x10000000 || address >= 0x10100000) {
        printf("STURB: Dirección 0x%016lX fuera de rango\n", address);
        return;
    }

    uint32_t word = mem_read_32(address & ~0x3);
    int byte_offset = address % 4;

    word &= ~(0xFF << (byte_offset * 8));
    word |= (byte_val << (byte_offset * 8));

    mem_write_32(address & ~0x3, word);

    printf("STURB: Guardando byte 0x%02X en la dirección 0x%016lX (Offset: %ld)\n", byte_val, address, offset);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_sturh(uint32_t instruction) {
    uint8_t rt = instruction & 0x1F;
    uint8_t rn = (instruction >> 5) & 0x1F;
    int64_t offset = ((instruction >> 12) & 0x2FF);

    if (offset & (1 << 11)) {
        offset |= 0xFFFFFFFFFFFFF000;
    }

    uint64_t address = CURRENT_STATE.REGS[rn] + offset;
    uint16_t halfword = CURRENT_STATE.REGS[rt] & 0xFFFF;

    uint32_t word = mem_read_32(address & ~0x3);
    int byte_offset = address % 4;

    word &= ~(0xFFFF << (byte_offset * 8));
    word |= (halfword << (byte_offset * 8));

    mem_write_32(address & ~0x3, word);

    printf("STURH: Guardando 0x%04X en la dirección 0x%016lX (Offset: %ld)\n", halfword, address, offset);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_ldur(uint32_t instruction) {
    uint8_t rt = instruction & 0x1F;
    uint8_t rn = (instruction >> 5) & 0x1F;
    int64_t offset = ((instruction >> 12) & 0x2FF);

    if (offset & (1 << 8)) {
        offset |= 0xFFFFFFFFFFFFFE00;
    }

    uint64_t address = CURRENT_STATE.REGS[rn] + offset;

    if (address < 0x10000000 || address + 7 >= 0x10100000) {
        printf("LDUR: Dirección 0x%016lX fuera de rango\n", address);
        return;
    }

    uint64_t lower = mem_read_32(address);
    uint64_t upper = mem_read_32(address + 4);
    uint64_t value = (upper << 32) | lower;
    
    NEXT_STATE.REGS[rt] = value;
    
    printf("LDUR: Cargando 0x%016lX desde la dirección 0x%016lX en X%d (Offset: %ld)\n", value, address, rt, offset);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_ldurb(uint32_t instruction) {
    uint8_t rt = instruction & 0x1F;
    uint8_t rn = (instruction >> 5) & 0x1F;
    int64_t offset = ((instruction >> 12) & 0x2FF);

    if (offset & (1 << 8)) {
        offset |= 0xFFFFFFFFFFFFFE00;
    }

    uint64_t address = CURRENT_STATE.REGS[rn] + offset;

    if (address < 0x10000000 || address >= 0x10100000) {
        printf("LDURB: Dirección 0x%016lX fuera de rango\n", address);
        return;
    }

    uint32_t word = mem_read_32(address & ~0x3);
    int byte_offset = address % 4;
    
    uint8_t byte_val = (word >> (byte_offset * 8)) & 0xFF;

    NEXT_STATE.REGS[rt] = byte_val;
    
    printf("LDURB: Cargando byte 0x%02X desde la dirección 0x%016lX en X%d (Offset: %ld)\n", byte_val, address, rt, offset);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_ldurh(uint32_t instruction) {
    uint8_t rt = instruction & 0x1F;
    uint8_t rn = (instruction >> 5) & 0x1F;
    int64_t offset = ((instruction >> 12) & 0x2FF);

    if (offset & (1 << 11)) {
        offset |= 0xFFFFFFFFFFFFF000;
    }

    uint64_t address = CURRENT_STATE.REGS[rn] + offset;

    uint32_t word = mem_read_32(address & ~0x3);
    int byte_offset = address % 4;
    
    uint16_t halfword = (word >> (byte_offset * 8)) & 0xFFFF;

    NEXT_STATE.REGS[rt] = (uint64_t)halfword & 0xFFFF;

    printf("LDURH: Cargando 0x%04X en W%d desde la dirección 0x%016lX (Offset: %ld)\n", halfword, rt, address, offset);

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_mul(uint32_t instruction) {
    uint8_t rd = instruction & 0x1F;
    uint8_t rn = (instruction >> 5) & 0x1F;
    uint8_t rm = (instruction >> 16) & 0x1F;

    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] * CURRENT_STATE.REGS[rm];

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_cbz(uint32_t instruction) {
    uint8_t rt = instruction & 0x1F;
    int32_t offset = (instruction >> 5) & 0x7FFFF;

    if (offset & (1 << 18)) {
        offset |= 0xFFF80000;
    }

    offset = offset << 2;

    if (CURRENT_STATE.REGS[rt] == 0) {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    } else { 
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
    CURRENT_STATE = NEXT_STATE;
}

void execute_cbnz(uint32_t instruction) {
    uint8_t rt = instruction & 0x1F;
    int32_t offset = (instruction >> 5) & 0x7FFFF;

    if (offset & (1 << 18)) {
        offset |= 0xFFF80000;
    }

    offset = offset << 2;

    if (CURRENT_STATE.REGS[rt] != 0) {
        NEXT_STATE.PC = CURRENT_STATE.PC + offset;
    } else { 
        NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    }
    CURRENT_STATE = NEXT_STATE;
}

void execute_add_immediate(uint32_t instruction) {
    uint8_t rd = instruction & 0x1F;
    uint8_t rn = (instruction >> 5) & 0x1F;
    uint16_t imm12 = (instruction >> 10) & 0xFFF;
    uint8_t shift = (instruction >> 22) & 1;
    uint64_t imm_value;

    if (shift == 1) {
        imm_value = imm12 << 12;
    } else {
        imm_value = imm12;
    }

    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] + imm_value;

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}

void execute_add_register(uint32_t instruction) {
    uint8_t rd = instruction & 0x1F;
    uint8_t rn = (instruction >> 5) & 0x1F;
    uint8_t rm = (instruction >> 16) & 0x1F;

    NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rn] + CURRENT_STATE.REGS[rm];

    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    CURRENT_STATE = NEXT_STATE;
}