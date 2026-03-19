#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#include "STATS.H"
#include "INVENTOR.H"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#define EQUIP_SLOTS EQUIP_NUMBER_OF_LOCATIONS

typedef struct {
    char **items;
    size_t count;
    size_t cap;
} PathList;

static const char *G_classNames[] = {
    "CITIZEN",
    "KNIGHT",
    "MAGE",
    "WARLOCK",
    "PRIEST",
    "ROGUE",
    "ARCHER",
    "SAILOR",
    "PALADIN",
    "MERCENARY",
    "MAGICIAN",
    "UNKNOWN"
};

static void die(const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "error: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(1);
}

static char *aa_strdup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *out = (char *)malloc(n);
    if (!out)
        die("out of memory");
    memcpy(out, s, n);
    return out;
}

static void pathlist_push(PathList *pl, const char *path)
{
    if (pl->count == pl->cap) {
        size_t ncap = (pl->cap == 0) ? 8 : pl->cap * 2;
        char **nitems = (char **)realloc(pl->items, ncap * sizeof(char *));
        if (!nitems)
            die("out of memory");
        pl->items = nitems;
        pl->cap = ncap;
    }
    pl->items[pl->count++] = aa_strdup(path);
}

static int starts_with(const char *s, const char *prefix)
{
    size_t n = strlen(prefix);
    return strncmp(s, prefix, n) == 0;
}

static int path_is_dir(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0)
        return 0;
    return S_ISDIR(st.st_mode) ? 1 : 0;
}

static int path_is_file(const char *path)
{
    struct stat st;
    if (stat(path, &st) != 0)
        return 0;
    return S_ISREG(st.st_mode) ? 1 : 0;
}

static void collect_chdata_from_dir(PathList *pl, const char *dirPath)
{
    DIR *dir = opendir(dirPath);
    struct dirent *ent;
    if (!dir)
        die("cannot open directory '%s': %s", dirPath, strerror(errno));

    while ((ent = readdir(dir)) != NULL) {
        char full[PATH_MAX];
        if (!starts_with(ent->d_name, "CHDATA"))
            continue;

        snprintf(full, sizeof(full), "%s/%s", dirPath, ent->d_name);
        if (path_is_file(full))
            pathlist_push(pl, full);
    }

    closedir(dir);
}

static void json_escape_bytes(FILE *out, const T_byte8 *bytes, size_t maxLen)
{
    size_t i;
    fputc('"', out);
    for (i = 0; i < maxLen; ++i) {
        unsigned char c = (unsigned char)bytes[i];
        if (c == '\0')
            break;
        switch (c) {
            case '"': fputs("\\\"", out); break;
            case '\\': fputs("\\\\", out); break;
            case '\b': fputs("\\b", out); break;
            case '\f': fputs("\\f", out); break;
            case '\n': fputs("\\n", out); break;
            case '\r': fputs("\\r", out); break;
            case '\t': fputs("\\t", out); break;
            default:
                if (c < 0x20) {
                    fprintf(out, "\\u%04x", (unsigned)c);
                } else {
                    fputc((int)c, out);
                }
                break;
        }
    }
    fputc('"', out);
}

static void json_string(FILE *out, const char *s)
{
    const unsigned char *p = (const unsigned char *)s;
    fputc('"', out);
    while (*p) {
        unsigned char c = *p++;
        switch (c) {
            case '"': fputs("\\\"", out); break;
            case '\\': fputs("\\\\", out); break;
            case '\b': fputs("\\b", out); break;
            case '\f': fputs("\\f", out); break;
            case '\n': fputs("\\n", out); break;
            case '\r': fputs("\\r", out); break;
            case '\t': fputs("\\t", out); break;
            default:
                if (c < 0x20) {
                    fprintf(out, "\\u%04x", (unsigned)c);
                } else {
                    fputc((int)c, out);
                }
                break;
        }
    }
    fputc('"', out);
}

static void dump_u8_array(FILE *out, const T_byte8 *arr, size_t n)
{
    size_t i;
    fputc('[', out);
    for (i = 0; i < n; ++i) {
        if (i)
            fputc(',', out);
        fprintf(out, "%u", (unsigned)arr[i]);
    }
    fputc(']', out);
}

static void dump_s16_array(FILE *out, const T_sword16 *arr, size_t n)
{
    size_t i;
    fputc('[', out);
    for (i = 0; i < n; ++i) {
        if (i)
            fputc(',', out);
        fprintf(out, "%d", (int)arr[i]);
    }
    fputc(']', out);
}

static void dump_u16_array(FILE *out, const T_word16 *arr, size_t n)
{
    size_t i;
    fputc('[', out);
    for (i = 0; i < n; ++i) {
        if (i)
            fputc(',', out);
        fprintf(out, "%u", (unsigned)arr[i]);
    }
    fputc(']', out);
}

static void dump_bool_array(FILE *out, const E_Boolean *arr, size_t n)
{
    size_t i;
    fputc('[', out);
    for (i = 0; i < n; ++i) {
        if (i)
            fputc(',', out);
        fprintf(out, "%u", (unsigned)arr[i]);
    }
    fputc(']', out);
}

static void dump_effect_data(FILE *out, T_word16 effectData[MAX_ITEM_EFFECTS][3])
{
    size_t i;
    fputc('[', out);
    for (i = 0; i < MAX_ITEM_EFFECTS; ++i) {
        if (i)
            fputc(',', out);
        fprintf(out, "[%u,%u,%u]",
                (unsigned)effectData[i][0],
                (unsigned)effectData[i][1],
                (unsigned)effectData[i][2]);
    }
    fputc(']', out);
}

static const char *class_name(T_byte8 classType)
{
    if (classType <= CLASS_MAGICIAN)
        return G_classNames[classType];
    return G_classNames[CLASS_UNKNOWN];
}

static void dump_player_stats(FILE *out, const T_playerStats *ps)
{
    size_t i;

    fputs("{\n", out);
    fputs("      \"Name\":", out); json_escape_bytes(out, ps->Name, sizeof(ps->Name)); fputs(",\n", out);
    fputs("      \"ClassName\":", out); json_escape_bytes(out, ps->ClassName, sizeof(ps->ClassName)); fputs(",\n", out);
    fputs("      \"ClassTitle\":", out); json_escape_bytes(out, ps->ClassTitle, sizeof(ps->ClassTitle)); fputs(",\n", out);

    fprintf(out, "      \"Health\":%d,\n", (int)ps->Health);
    fprintf(out, "      \"MaxHealth\":%d,\n", (int)ps->MaxHealth);
    fprintf(out, "      \"Mana\":%d,\n", (int)ps->Mana);
    fprintf(out, "      \"MaxMana\":%d,\n", (int)ps->MaxMana);

    fprintf(out, "      \"Food\":%d,\n", (int)ps->Food);
    fprintf(out, "      \"MaxFood\":%d,\n", (int)ps->MaxFood);
    fprintf(out, "      \"Water\":%d,\n", (int)ps->Water);
    fprintf(out, "      \"MaxWater\":%d,\n", (int)ps->MaxWater);

    fprintf(out, "      \"PoisonLevel\":%d,\n", (int)ps->PoisonLevel);
    fprintf(out, "      \"RegenHealth\":%d,\n", (int)ps->RegenHealth);
    fprintf(out, "      \"RegenMana\":%d,\n", (int)ps->RegenMana);

    fprintf(out, "      \"JumpPower\":%u,\n", (unsigned)ps->JumpPower);
    fprintf(out, "      \"JumpPowerMod\":%u,\n", (unsigned)ps->JumpPowerMod);
    fprintf(out, "      \"Tallness\":%u,\n", (unsigned)ps->Tallness);
    fprintf(out, "      \"ClimbHeight\":%u,\n", (unsigned)ps->ClimbHeight);
    fprintf(out, "      \"MaxVRunning\":%u,\n", (unsigned)ps->MaxVRunning);
    fprintf(out, "      \"MaxVWalking\":%u,\n", (unsigned)ps->MaxVWalking);

    fprintf(out, "      \"HeartRate\":%u,\n", (unsigned)ps->HeartRate);
    fprintf(out, "      \"MaxFallV\":%u,\n", (unsigned)ps->MaxFallV);

    fprintf(out, "      \"WeaponBaseDamage\":%u,\n", (unsigned)ps->WeaponBaseDamage);
    fprintf(out, "      \"WeaponBaseSpeed\":%d,\n", (int)ps->WeaponBaseSpeed);
    fprintf(out, "      \"AttackSpeed\":%u,\n", (unsigned)ps->AttackSpeed);
    fprintf(out, "      \"AttackDamage\":%u,\n", (unsigned)ps->AttackDamage);

    fprintf(out, "      \"playerisalive\":%u,\n", (unsigned)ps->playerisalive);
    fprintf(out, "      \"ClassType\":%u,\n", (unsigned)ps->ClassType);
    fputs("      \"ClassTypeName\":", out); json_string(out, class_name(ps->ClassType)); fputs(",\n", out);

    fputs("      \"Attributes\":", out); dump_u8_array(out, ps->Attributes, NUM_ATTRIBUTES); fputs(",\n", out);
    fputs("      \"AttributeMods\":", out); dump_s16_array(out, ps->AttributeMods, NUM_ATTRIBUTES); fputs(",\n", out);
    fputs("      \"Coins\":", out); dump_s16_array(out, ps->Coins, EQUIP_TOTAL_COIN_TYPES); fputs(",\n", out);
    fputs("      \"Bolts\":", out); dump_s16_array(out, ps->Bolts, EQUIP_TOTAL_BOLT_TYPES); fputs(",\n", out);
    fputs("      \"SavedCoins\":", out); dump_s16_array(out, ps->SavedCoins, EQUIP_TOTAL_COIN_TYPES); fputs(",\n", out);

    fputs("      \"ArmorValues\":", out); dump_u8_array(out, ps->ArmorValues, EQUIP_NUMBER_OF_LOCATIONS); fputs(",\n", out);
    fprintf(out, "      \"ArmorLevel\":%u,\n", (unsigned)ps->ArmorLevel);
    fprintf(out, "      \"Load\":%u,\n", (unsigned)ps->Load);
    fprintf(out, "      \"MaxLoad\":%u,\n", (unsigned)ps->MaxLoad);

    fprintf(out, "      \"Level\":%u,\n", (unsigned)ps->Level);
    fprintf(out, "      \"Experience\":%u,\n", (unsigned)ps->Experience);
    fprintf(out, "      \"ExpNeeded\":%u,\n", (unsigned)ps->ExpNeeded);

    fprintf(out, "      \"SpellSystem\":%u,\n", (unsigned)ps->SpellSystem);
    fputs("      \"ActiveRunes\":", out); dump_u8_array(out, ps->ActiveRunes, 9); fputs(",\n", out);

    fputs("      \"HouseOwned\":", out); dump_bool_array(out, ps->HouseOwned, NUM_HOUSES); fputs(",\n", out);
    fputs("      \"HasNotes\":", out); dump_u8_array(out, ps->HasNotes, (MAX_NOTES / 8) + 1); fputs(",\n", out);
    fprintf(out, "      \"NumNotes\":%u,\n", (unsigned)ps->NumNotes);

    /* TODO: add CLI flags to optionally include large arrays (Notes/Identified). */
    fprintf(out, "      \"NotesOmitted\":true,\n");
    fprintf(out, "      \"NotesLength\":%u,\n", (unsigned)MAX_NOTE_SIZE);
    fprintf(out, "      \"IdentifiedOmitted\":true,\n");
    fprintf(out, "      \"IdentifiedLength\":%u,\n", 8193u);
    fputs("      \"password\":", out); dump_u8_array(out, ps->password, MAX_SIZE_PASSWORD); fputs(",\n", out);

    fprintf(out, "      \"CompletedAdventure\":%u,\n", (unsigned)ps->CompletedAdventure);
    fprintf(out, "      \"CompletedMap\":%u,\n", (unsigned)ps->CompletedMap);
    fprintf(out, "      \"CurrentQuestNumber\":%u,\n", (unsigned)ps->CurrentQuestNumber);

    fprintf(out, "      \"pastPlaces\":{\n");
    fprintf(out, "        \"numInList\":%u,\n", (unsigned)ps->pastPlaces.numInList);
    fputs("        \"places\":[\n", out);
    for (i = 0; i < MAX_PAST_PLACES; ++i) {
        if (i)
            fputs(",\n", out);
        fprintf(out,
                "          {\"adventureNumber\":%u,\"lastLevelNumber\":%u}",
                (unsigned)ps->pastPlaces.places[i].adventureNumber,
                (unsigned)ps->pastPlaces.places[i].lastLevelNumber);
    }
    fputs("\n        ]\n", out);
    fputs("      }\n", out);
    fputs("    }", out);
}

static void dump_itemdesc(FILE *out, const T_equipItemDescription *d)
{
    fputs("{\n", out);
    fprintf(out, "          \"type\":%u,\n", (unsigned)d->type);
    fprintf(out, "          \"subtype\":%u,\n", (unsigned)d->subtype);
    fprintf(out, "          \"numstackable\":%u,\n", (unsigned)d->numstackable);

    fputs("          \"effectTriggerOn\":", out); dump_u8_array(out, d->effectTriggerOn, MAX_ITEM_EFFECTS); fputs(",\n", out);
    fputs("          \"effectType\":", out); dump_u8_array(out, d->effectType, MAX_ITEM_EFFECTS); fputs(",\n", out);
    fputs("          \"effectData\":", out); dump_effect_data(out, (T_word16 (*)[3])d->effectData); fputs(",\n", out);

    fprintf(out, "          \"objectDestroyOn\":%u,\n", (unsigned)d->objectDestroyOn);
    fprintf(out, "          \"useable\":%u,\n", (unsigned)d->useable);
    fprintf(out, "          \"unique\":%u\n", (unsigned)d->unique);
    fputs("        }", out);
}

static void dump_inventory_record(FILE *out, const T_inventoryItemStruct *it, size_t idx)
{
    fputs("      {\n", out);
    fprintf(out, "        \"recordIndex\":%zu,\n", idx);
    fprintf(out, "        \"equippedSlotIndex\":%d,\n", (idx < EQUIP_SLOTS) ? (int)idx : -1);
    fprintf(out, "        \"locx\":%u,\n", (unsigned)it->locx);
    fprintf(out, "        \"locy\":%u,\n", (unsigned)it->locy);
    fprintf(out, "        \"picwidth\":%u,\n", (unsigned)it->picwidth);
    fprintf(out, "        \"picheight\":%u,\n", (unsigned)it->picheight);
    fprintf(out, "        \"gridstartx\":%d,\n", (int)it->gridstartx);
    fprintf(out, "        \"gridstarty\":%d,\n", (int)it->gridstarty);
    fprintf(out, "        \"gridspacesx\":%d,\n", (int)it->gridspacesx);
    fprintf(out, "        \"gridspacesy\":%d,\n", (int)it->gridspacesy);
    fprintf(out, "        \"object\":\"0x%" PRIxPTR "\",\n", (uintptr_t)it->object);
    fprintf(out, "        \"objecttype\":%u,\n", (unsigned)it->objecttype);
    fprintf(out, "        \"p_bitmap\":\"0x%" PRIxPTR "\",\n", (uintptr_t)it->p_bitmap);
    fprintf(out, "        \"storepage\":%u,\n", (unsigned)it->storepage);
    fprintf(out, "        \"numitems\":%u,\n", (unsigned)it->numitems);
    fputs("        \"itemdesc\":", out);
    dump_itemdesc(out, &it->itemdesc);
    fprintf(out, ",\n        \"elementID\":\"0x%" PRIxPTR "\"\n", (uintptr_t)it->elementID);
    fputs("      }", out);
}

static void dump_one_file(FILE *out, const char *path)
{
    FILE *fp;
    T_playerStats stats;
    T_inventoryItemStruct rec;
    size_t recIndex = 0;
    int first = 1;

    fp = fopen(path, "rb");
    if (!fp)
        die("cannot open '%s': %s", path, strerror(errno));

    if (fread(&stats, sizeof(stats), 1, fp) != 1) {
        fclose(fp);
        die("'%s' is too small to contain T_playerStats (%zu bytes)", path, sizeof(stats));
    }

    fputs("  {\n", out);
    fputs("    \"path\":", out); json_string(out, path); fputs(",\n", out);
    fprintf(out, "    \"playerStatsSize\":%zu,\n", sizeof(T_playerStats));
    fprintf(out, "    \"inventoryRecordSize\":%zu,\n", sizeof(T_inventoryItemStruct));
    fputs("    \"playerStats\":", out);
    dump_player_stats(out, &stats);
    fputs(",\n", out);
    fputs("    \"inventoryRecords\":[\n", out);

    /* Read equipped items first (EQUIP_NUMBER_OF_LOCATIONS = 15) */
    for (int i = 0; i < 15; ++i) {
        if (fread(&rec, sizeof(rec), 1, fp) != 1)
            break;
        
        /* Only dump non-blank equipped items */
        if (rec.objecttype != 0) {
            if (!first)
                fputs(",\n", out);
            first = 0;
            dump_inventory_record(out, &rec, recIndex);
            ++recIndex;
        }
    }

    /* Read remaining inventory records until we hit a blank record (objecttype == 0) */
    while (fread(&rec, sizeof(rec), 1, fp) == 1) {
        /* Blank record (objecttype == 0) terminates the inventory list */
        if (rec.objecttype == 0)
            break;
        
        if (!first)
            fputs(",\n", out);
        first = 0;
        dump_inventory_record(out, &rec, recIndex);
        ++recIndex;
    }

    if (ferror(fp)) {
        fclose(fp);
        die("read error while reading inventory records from '%s'", path);
    }

    fprintf(out, "\n    ],\n    \"inventoryRecordCount\":%zu\n", recIndex);
    fputs("  }", out);

    fclose(fp);
}

int main(int argc, char **argv)
{
    PathList list = {0};
    FILE *out = stdout;
    int argi = 1;
    size_t i;

    if (argc < 2) {
        fprintf(stderr,
                "Usage: %s [-o output.json] <CHDATA file|directory> [more files/dirs...]\n",
                argv[0]);
        return 2;
    }

    if (argc >= 4 && strcmp(argv[1], "-o") == 0) {
        out = fopen(argv[2], "wb");
        if (!out)
            die("cannot open output '%s': %s", argv[2], strerror(errno));
        argi = 3;
    }

    for (; argi < argc; ++argi) {
        const char *p = argv[argi];
        if (path_is_dir(p)) {
            collect_chdata_from_dir(&list, p);
        } else if (path_is_file(p)) {
            pathlist_push(&list, p);
        } else {
            die("path not found or not a regular file/directory: '%s'", p);
        }
    }

    if (list.count == 0)
        die("no CHDATA files found");

    fputs("{\n", out);
    fprintf(out, "  \"schemaVersion\":1,\n");
    fprintf(out, "  \"sizeof\":{\"T_playerStats\":%zu,\"T_inventoryItemStruct\":%zu},\n",
            sizeof(T_playerStats),
            sizeof(T_inventoryItemStruct));
    fputs("  \"characters\":[\n", out);

    for (i = 0; i < list.count; ++i) {
        if (i)
            fputs(",\n", out);
        dump_one_file(out, list.items[i]);
    }

    fputs("\n  ]\n}\n", out);

    if (out != stdout)
        fclose(out);

    for (i = 0; i < list.count; ++i)
        free(list.items[i]);
    free(list.items);

    return 0;
}
