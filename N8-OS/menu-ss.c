
#include "everdrive.h"

void app_inGameMenu();

void inGameMenu() {

    u8 bank = REG_APP_BANK;
    REG_APP_BANK = APP_SS;
    app_inGameMenu();
    REG_APP_BANK = bank;

}


#pragma codeseg ("BNK08")

void ss_return();

enum {
    SS_SAVE,
    SS_LOAD,
    SS_BANK,
    SS_EXIT,
    SS_SWAP_DISK,
    SS_SIZE
};

void app_inGameMenu() {

    u8 resp;
    FileInfo inf = {0};
    ListBox box;
    u8 * items[SS_SIZE + 1];
    u8 buff[16];
    u8 ss_src;
    u8 ss_bank_hex;
    u8 update_info = 1;

    edInit(1);
    

    REG_SST_ADDR = 0xff; //ss hit byte
    ss_src = REG_SST_DATA;

    //mem_set(&box, 0, sizeof (ListBox));
    box.hdr = 0;
    box.items = items;
    box.selector = 0;
    items[SS_SAVE] = "Save State";
    items[SS_LOAD] = "Load State";
    items[SS_EXIT] = "Exit Game";
    items[SS_SWAP_DISK] = 0; //"Swap Disk";
    items[SS_SIZE] = 0;

    ss_bank_hex = decToBcd(ses_cfg->ss_bank);

    //quick ss section
    if (
            registery->options.ss_mode == SS_MOD_QSS &&
            registery->options.ss_key_load != registery->options.ss_key_save) {

        if (ss_src == SS_SRC_JOY_LOAD) {
            ppuOFF();
            resp = srmRestoreSS(ss_bank_hex);
            if (resp)printError(resp);
            ss_return();
        }

        if (ss_src == SS_SRC_JOY_SAVE) {
            ppuOFF();
            resp = srmBackupSS(ss_bank_hex);
            if (resp)printError(resp);
            ss_return();
        }
    }

    while (1) {

        if (update_info) {
            resp = srmGetInfoSS(&inf, ss_bank_hex);
            update_info = 0;
        }

        gSetPal(PAL_G2);
        if (resp == 0) {
            gDrawFooter("Save Time: ", 1, 0);
            gAppendDate(inf.date);
            gAppendString(" ");
            gAppendTime(inf.time);
        } else {
            gDrawFooter("Empty Slot", 1, G_CENTER);
        }

        buff[0] = 0;
        str_append(buff, "Slot: ");
        str_append_hex8(buff, ss_bank_hex);
        items[SS_BANK] = buff;

        box.selector |= SEL_DPD;
        guiDrawListBox(&box);

        if (box.act == ACT_EXIT) {
            ss_return();
        }


        if (box.act == JOY_L) {
            ses_cfg->ss_bank = dec_mod(ses_cfg->ss_bank, MAX_SS_SLOTS);
            ss_bank_hex = decToBcd(ses_cfg->ss_bank); 
            update_info = 1;
        }

        if (box.act == JOY_R) {
            ses_cfg->ss_bank = inc_mod(ses_cfg->ss_bank, MAX_SS_SLOTS);
            ss_bank_hex = decToBcd(ses_cfg->ss_bank);
            update_info = 1;
        }


        //gCleanScreen();
        if (box.selector == SS_BANK)continue;

        if (box.act == ACT_OPEN)break;
    }

    ppuOFF();

    if (box.selector == SS_SWAP_DISK) {
        REG_FDS_SWAP = 1;
        ss_return();
    }

    if (box.selector == SS_SAVE) {
        resp = srmBackupSS(ss_bank_hex);
        if (resp)printError(resp);
        ss_return();
    }

    if (box.selector == SS_LOAD) {
        resp = srmRestoreSS(ss_bank_hex);
        if (resp)printError(resp);
        ss_return();
    }

    if (box.selector == SS_EXIT) {
        bi_exit_game();
    }

}
