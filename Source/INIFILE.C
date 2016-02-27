/*-------------------------------------------------------------------------*
 * File:  INIFILE.C
 *-------------------------------------------------------------------------*/
/**
 * A system for opening, reading, editing, and saving .INI files.
 *
 * @addtogroup INIFILE
 * @brief INI File System
 * @see http://www.amuletsandarmor.com/AALicense.txt
 * @{
 *
 *<!-----------------------------------------------------------------------*/
#include "DBLLINK.H"
#include "INIFILE.H"
#include "MEMORY.H"
#include <ctype.h>

#define INIFILE_TAG               (*((T_word32 *)"iNiF"))
#define INIFILE_DEAD_TAG          (*((T_word32 *)"DinI"))

typedef struct {
    T_byte8 key[32] ;
    T_byte8 value[80] ;
} T_iniKey ;

typedef struct {
    T_byte8 name[32] ;
    T_doubleLinkList keyList ;
} T_iniCategory ;

typedef struct {
    T_word32 tag ;
    T_doubleLinkList categoryList ;
    E_Boolean isDirty ;
} T_iniFileStruct ;

/* Internal prototypes: */
static T_iniCategory *ICreateCategory(
                          T_iniFileStruct *p_ini,
                          T_byte8 *p_catName) ;

static T_iniKey *ICreateKey(
                     T_iniCategory *p_category,
                     T_byte8 *p_keyName) ;

static T_iniCategory *IFindCategory(
                          T_iniFileStruct *p_ini,
                          T_byte8 *p_catName) ;

static T_iniKey *IFindKey(
                          T_iniCategory *p_category,
                          T_byte8 *p_keyName) ;

static T_iniKey *IFindKeyAndElement(
                          T_iniCategory *p_category,
                          T_byte8 *p_keyName,
                          T_doubleLinkListElement *p_element) ;

static T_void IINIDestroy(T_iniFileStruct *p_ini) ;

static T_void IINIFilePutAlways(
           T_iniFile iniFile,
           T_byte8 *p_catName,
           T_byte8 *p_keyName,
           T_byte8 *p_value) ;
/* Quick access macros: */
#define IMarkDirty(p_ini)  ((p_ini)->isDirty = TRUE)
#define IIsDirty(p_ini)  ((p_ini)->isDirty)
#define IMarkClean(p_ini)  ((p_ini)->isDirty = FALSE)

/*-------------------------------------------------------------------------*
 * Routine:  INIFileOpen
 *-------------------------------------------------------------------------*/
/**
 *  INIFileOpen opens up an ini file for future accesses.
 *
 *  @param p_filename -- File name to save to (if changes)
 *
 *  @return Handle to ini file
 *
 *<!-----------------------------------------------------------------------*/
T_iniFile INIFileOpen(T_byte8 *p_filename)
{
    T_iniFile iniFile = INIFILE_BAD ;
    T_byte8 category[40] ;
    T_byte8 key[40] ;
    FILE *fp ;
    T_iniFileStruct *p_ini ;
    T_byte8  buffer[160] ;
    T_byte8 *p_char ;

    DebugRoutine("INIFileOpen") ;

    /* Open the .ini file */
    fp = fopen(p_filename, "r") ;
//    DebugCheck(fp != NULL) ;

    /* Allocate memory for the handle. */
    p_ini = MemAlloc(sizeof(T_iniFileStruct)) ;
    DebugCheck(p_ini) ;

    /* Make sure both are ok. */
    if (/* (fp != NULL) && */(p_ini != NULL))  {
        p_ini->tag = INIFILE_TAG ;
        p_ini->categoryList = DoubleLinkListCreate() ;

        if (fp != NULL)  {
            while (!feof(fp))  {
                buffer[0] = '\0' ;
				fgets(buffer, 160, fp);

#if (_MSC_VER == 1800)
				if (strlen(buffer) == 0)
					break;
#endif

                buffer[strlen(buffer)-1] = '\0' ;

                if (isalnum(buffer[0]))  {
                    /* Break it up into two parts, */
                    /* before equal, and after equal. */
                    p_char = strstr(buffer, "=") ;
                    if (p_char)  {
                        *p_char = '\0' ;
                        sscanf(buffer, "%s", key) ;
//                        sscanf(p_char+1, "%s", value) ;
                        IINIFilePutAlways(
                            (T_iniFile)p_ini,
                            category,
                            key,
                            p_char+2) ;
                    }
                } else if (buffer[0] == '[')  {
                    strcpy(category, buffer+1) ;
                    p_char = strstr(category, "]") ;
                    if (p_char)
                        *p_char = '\0' ;
                    if (IFindCategory(p_ini, category) == NULL)
                        ICreateCategory(p_ini, category) ;
                }
            }
        }

        /* Note that no changes have occured (yet) */
        IMarkClean(p_ini) ;

        iniFile = (T_iniFile)p_ini ;
    }

    if (fp)
        fclose(fp) ;

    DebugEnd() ;

    return iniFile ;
}

/*-------------------------------------------------------------------------*
 * Routine:  INIFileClose
 *-------------------------------------------------------------------------*/
/**
 *  INIFileClose closes out the ini file and if there are changes, saves
 *  the file to the given p_filename.
 *
 *  @param p_filename -- File name to save to (if changes)
 *  @param iniFile -- IniFile to search
 *
 *<!-----------------------------------------------------------------------*/
T_void INIFileClose(T_byte8 *p_filename, T_iniFile iniFile)
{
    T_iniFileStruct *p_ini ;
    T_iniCategory *p_cat ;
    T_iniKey *p_key ;
    FILE *fp ;

    T_doubleLinkListElement elementCat ;
    T_doubleLinkListElement elementKey ;

    DebugRoutine("INIFileClose") ;

    /* Get access to the ini data. */
    p_ini = (T_iniFileStruct *)iniFile ;
    DebugCheck(p_ini->tag == INIFILE_TAG) ;

    /* Check to see if we need to save out changes. */
    if (IIsDirty(p_ini))  {
        /* somebody has made a change and thus we need */
        /* to save out that change. */

        /* Open up a file to save this to. */
        fp = fopen(p_filename, "w") ;

        /* Save out each category in order that we found them. */
        elementCat = DoubleLinkListGetFirst(p_ini->categoryList) ;
        while (elementCat != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
            p_cat = (T_iniCategory *)DoubleLinkListElementGetData(
                                         elementCat) ;
            fprintf(fp, "\n[%s]\n", p_cat->name) ;

            elementKey = DoubleLinkListGetFirst(p_cat->keyList) ;

            /* Loop through all the keys and output. */
            while (elementKey != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
                p_key = DoubleLinkListElementGetData(elementKey) ;
                fprintf(fp, "%s = %s\n", p_key->key, p_key->value) ;

                elementKey = DoubleLinkListElementGetNext(elementKey) ;
            }

            elementCat = DoubleLinkListElementGetNext(elementCat) ;
        }

        /* Add a couple of blank lines for good measure. */
        fprintf(fp, "\n\n") ;

        /* Close out the .ini file. */
        fclose(fp);
    }

    IINIDestroy(p_ini) ;

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  INIFileGet
 *-------------------------------------------------------------------------*/
/**
 *  INIFileGet searches for a key based on its category.
 *
 *  @param iniFile -- IniFile to search
 *  @param p_catName -- Search category
 *  @param p_keyName -- Search key
 *
 *  @return Value found, else NULL
 *
 *<!-----------------------------------------------------------------------*/
T_byte8 *INIFileGet(
             T_iniFile iniFile,
             T_byte8 *p_catName,
             T_byte8 *p_keyName)
{
    T_iniFileStruct *p_ini ;
    T_iniCategory *p_category ;
    T_iniKey *p_key ;
    T_byte8 *p_found = NULL ;
    T_word16 i ;

    DebugRoutine("INIFileGet") ;

    /* Get access to the ini data. */
    p_ini = (T_iniFileStruct *)iniFile ;
    DebugCheck(p_ini->tag == INIFILE_TAG) ;

    /* Don't bother if this is a bad ini file. */
    if (p_ini)  {
        /* First, find the category. */
        p_category = IFindCategory(p_ini, p_catName) ;
        if (p_category)  {
            /* If we found category, find the key within. */
            p_key = IFindKey(p_category, p_keyName) ;

            /* If found, return the value. */
            if (p_key)  {
                p_found = p_key->value ;

                /* Make underbars spaces. */
                for (i=0; p_found[i] != '\0'; i++)
                    if (p_found[i] == '_')
                        p_found[i] = ' ' ;
            }
        }
    }

    DebugEnd() ;

    return p_found ;
}

/*-------------------------------------------------------------------------*
 * Routine:  INIFilePut
 *-------------------------------------------------------------------------*/
/**
 *  INIFilePut adds or changes a category, key, and value combo.  It
 *  basically does it all to add or change an entry.
 *
 *  @param iniFile -- IniFile to put entry
 *  @param p_catName -- Name of category
 *  @param p_keyName -- Name of key
 *  @param p_value -- Value to use
 *
 *<!-----------------------------------------------------------------------*/
T_void INIFilePut(
           T_iniFile iniFile,
           T_byte8 *p_catName,
           T_byte8 *p_keyName,
           T_byte8 *p_value)
{
    T_iniFileStruct *p_ini ;
    T_iniCategory *p_category ;
    T_iniKey *p_key ;
    T_word16 i ;

    DebugRoutine("INIFilePut") ;

    /* Get access to the ini data. */
    p_ini = (T_iniFileStruct *)iniFile ;
    DebugCheck(p_ini->tag == INIFILE_TAG) ;

    /* Don't bother if this is a bad ini file. */
    if (p_ini)  {
        /* First, find the category. */
        p_category = IFindCategory(p_ini, p_catName) ;
        if (!p_category)
            p_category = ICreateCategory(p_ini, p_catName) ;

        if (p_category)  {
            /* If we found category, find the key within. */
            p_key = IFindKey(p_category, p_keyName) ;

            if (!p_key)
                p_key = ICreateKey(p_category, p_keyName) ;

            if (p_key)  {
                /* Modify the value */
                strcpy(p_key->value, p_value) ;

                /* Make spaces underbars. */
                for (i=0; p_key->value[i] != '\0'; i++)
                    if (isspace(p_key->value[i]))
                        p_key->value[i] = '_' ;

                IMarkDirty(p_ini) ;
            }
        }
    }

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  INIFilePut
 *-------------------------------------------------------------------------*/
/**
 *  INIFilePut adds or changes a category, key, and value combo.  It
 *  basically does it all to add or change an entry.
 *
 *  @param iniFile -- IniFile to put entry
 *  @param p_catName -- Name of category
 *  @param p_keyName -- Name of key
 *  @param p_value -- Value to use
 *
 *<!-----------------------------------------------------------------------*/
static T_void IINIFilePutAlways(
           T_iniFile iniFile,
           T_byte8 *p_catName,
           T_byte8 *p_keyName,
           T_byte8 *p_value)
{
    T_iniFileStruct *p_ini ;
    T_iniCategory *p_category ;
    T_iniKey *p_key ;
    T_word16 i ;

    DebugRoutine("INIFilePut") ;

    /* Get access to the ini data. */
    p_ini = (T_iniFileStruct *)iniFile ;
    DebugCheck(p_ini->tag == INIFILE_TAG) ;

    /* Don't bother if this is a bad ini file. */
    if (p_ini)  {
        /* First, find the category. */
        p_category = IFindCategory(p_ini, p_catName) ;
        if (!p_category)
            p_category = ICreateCategory(p_ini, p_catName) ;

        if (p_category)  {
            /* Always create the new field */
            p_key = ICreateKey(p_category, p_keyName) ;

            if (p_key)  {
                /* Modify the value */
                strcpy(p_key->value, p_value) ;

                /* Make spaces underbars. */
                for (i=0; p_key->value[i] != '\0'; i++)
                    if (isspace(p_key->value[i]))
                        p_key->value[i] = '_' ;

                IMarkDirty(p_ini) ;
            }
        }
    }

    DebugEnd() ;
}


/*-------------------------------------------------------------------------*
 * Routine:  ICreateCategory
 *-------------------------------------------------------------------------*/
/**
 *  ICreateCategory creates a new category with the given name and
 *  attaches it at the end of the category list.
 *
 *  @param p_ini -- IniFile to create category
 *  @param p_catName -- Name of category to create
 *
 *  @return Craete category, else NULL
 *
 *<!-----------------------------------------------------------------------*/
static T_iniCategory *ICreateCategory(
                          T_iniFileStruct *p_ini,
                          T_byte8 *p_catName)
{
    T_iniCategory *p_category ;

    DebugRoutine("ICreateCategory") ;

    /* Allocte memory for the category */
    p_category = MemAlloc(sizeof(T_iniCategory)) ;
    DebugCheck(p_category != NULL) ;
    if (p_category)  {
        /* Copy in the name. */
        strcpy(p_category->name, p_catName) ;

        /* Create a key list. */
        p_category->keyList = DoubleLinkListCreate() ;

        /* Finally, add the category to the end of the list. */
        DoubleLinkListAddElementAtEnd(p_ini->categoryList, p_category) ;
    }

    DebugEnd() ;

    return p_category ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IFindCategory
 *-------------------------------------------------------------------------*/
/**
 *  IFindCategory searches a iniFile's list of categories for match.
 *
 *  @param p_ini -- Ini file to search
 *  @param p_catName -- Name of category to find
 *
 *  @return Found category, else NULL
 *
 *<!-----------------------------------------------------------------------*/
static T_iniCategory *IFindCategory(
                          T_iniFileStruct *p_ini,
                          T_byte8 *p_catName)
{
    T_doubleLinkListElement element ;
    T_iniCategory *p_category = NULL ;

    DebugRoutine("IFindCategory") ;

    /* Go through the list of categories and try to find */
    /* a matching category. */
    element = DoubleLinkListGetFirst(p_ini->categoryList) ;
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        /* Look up the categories data. */
        p_category = (T_iniCategory *)DoubleLinkListElementGetData(element) ;

        if (strcmp(p_category->name, p_catName) == 0)
            break ;

        /* Not here, null out this case. */
        p_category = NULL ;

        /* Move to the next element in the category list. */
        element = DoubleLinkListElementGetNext(element) ;
    }

    DebugEnd() ;

    return p_category ;
}


/*-------------------------------------------------------------------------*
 * Routine:  IFindKey
 *-------------------------------------------------------------------------*/
/**
 *  IFindKey searches a category's list of keys for a match.
 *
 *  @param p_category -- Category to search in
 *  @param p_keyName -- Name of key to find
 *
 *  @return Found key, else NULL
 *
 *<!-----------------------------------------------------------------------*/
static T_iniKey *IFindKey(
                          T_iniCategory *p_category,
                          T_byte8 *p_keyName)
{
    T_doubleLinkListElement element ;
    T_iniKey *p_key = NULL ;

    DebugRoutine("IFindKey") ;

    /* Go through the list of keys for this category and try to find */
    /* a match. */
    element = DoubleLinkListGetFirst(p_category->keyList) ;
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        /* Look up the key data. */
        p_key = (T_iniKey *)DoubleLinkListElementGetData(element) ;

        /* Stop here if we found a match. */
        if (strcmp(p_key->key, p_keyName) == 0)
            break ;

        /* Otherwise, not here, null out this case. */
        p_key = NULL ;

        /* Move to the next element in the category list. */
        element = DoubleLinkListElementGetNext(element) ;
    }

    DebugEnd() ;

    return p_key ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IFindKeyAndElement
 *-------------------------------------------------------------------------*/
/**
 *  IFindKey searches a category's list of keys for a match.
 *
 *  @param p_category -- Category to search in
 *  @param p_keyName -- Name of key to find
 *
 *  @return Found key, else NULL
 *
 *<!-----------------------------------------------------------------------*/
static T_iniKey *IFindKeyAndElement(
                          T_iniCategory *p_category,
                          T_byte8 *p_keyName,
                          T_doubleLinkListElement *p_element)
{
    T_doubleLinkListElement element ;
    T_iniKey *p_key = NULL ;

    DebugRoutine("IFindKey") ;

    /* Go through the list of keys for this category and try to find */
    /* a match. */
    *p_element = DOUBLE_LINK_LIST_ELEMENT_BAD ;
    element = DoubleLinkListGetFirst(p_category->keyList) ;
    while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        /* Look up the key data. */
        p_key = (T_iniKey *)DoubleLinkListElementGetData(element) ;

        /* Stop here if we found a match. */
        if (strcmp(p_key->key, p_keyName) == 0)  {
            *p_element = element ;
            break ;
        }

        /* Otherwise, not here, null out this case. */
        p_key = NULL ;

        /* Move to the next element in the category list. */
        element = DoubleLinkListElementGetNext(element) ;
    }

    DebugEnd() ;

    return p_key ;
}

/*-------------------------------------------------------------------------*
 * Routine:  ICreateKey
 *-------------------------------------------------------------------------*/
/**
 *  ICreateKey allocs memory for a key, sets up its name, and adds it
 *  to the given category.
 *
 *  @param p_category -- Category to add key to
 *  @param p_keyName -- Name of key
 *
 *  @return Created key
 *
 *<!-----------------------------------------------------------------------*/
static T_iniKey *ICreateKey(
                     T_iniCategory *p_category,
                     T_byte8 *p_keyName)
{
    T_iniKey *p_key ;

    DebugRoutine("ICreateKey") ;

    /* Allocte memory for the key */
    p_key = MemAlloc(sizeof(T_iniKey)) ;
    DebugCheck(p_key != NULL) ;
    if (p_key)  {
        /* Set up the key. */
        strcpy(p_key->key, p_keyName) ;
        p_key->value[0] = '\0' ;

        /* Finally, add the key to the end of the list. */
        DoubleLinkListAddElementAtEnd(p_category->keyList, p_key) ;
    }

    DebugEnd() ;

    return p_key ;
}

/*-------------------------------------------------------------------------*
 * Routine:  IINIDestroy
 *-------------------------------------------------------------------------*/
/**
 *  IINIDestroy frees and destroys the given ini file.
 *
 *  @param p_ini -- INIFile to destroy
 *
 *<!-----------------------------------------------------------------------*/
static T_void IINIDestroy(T_iniFileStruct *p_ini)
{
    T_iniCategory *p_cat ;
    T_doubleLinkListElement elementCat ;
    T_doubleLinkListElement elementKey ;

    DebugRoutine("IINIDestroy") ;

    elementCat = DoubleLinkListGetFirst(p_ini->categoryList) ;
    while (elementCat != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
        p_cat = (T_iniCategory *)DoubleLinkListElementGetData(elementCat) ;

        elementKey = DoubleLinkListGetFirst(p_cat->keyList) ;
        while (elementKey != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
            MemFree(DoubleLinkListElementGetData(elementKey)) ;
            DoubleLinkListRemoveElement(elementKey) ;
            elementKey = DoubleLinkListGetFirst(p_cat->keyList) ;
        }

        DoubleLinkListDestroy(p_cat->keyList) ;
        p_cat->keyList = DOUBLE_LINK_LIST_BAD ;
        MemFree(p_cat) ;

        DoubleLinkListRemoveElement(elementCat) ;
        elementCat = DoubleLinkListGetFirst(p_ini->categoryList) ;
    }

    DoubleLinkListDestroy(p_ini->categoryList) ;
    p_ini->categoryList = DOUBLE_LINK_LIST_BAD ;

    /* Mark the ini file as destroyed. */
    p_ini->tag = INIFILE_DEAD_TAG ;

    MemFree(p_ini) ;

    DebugEnd() ;
}

/*-------------------------------------------------------------------------*
 * Routine:  INIFileGetString
 *-------------------------------------------------------------------------*/
/**
 *  INIFileGetString gets a string for a given key and category.  If
 *  the entry cannot be found, a null string is returned.
 *
 *  @param iniFile -- IniFile to search
 *  @param p_catName -- Search category
 *  @param p_keyName -- Search key
 *  @param p_string -- Where to put string
 *  @param maxLength -- Maximum number characters in string
 *
 *<!-----------------------------------------------------------------------*/
T_void INIFileGetString(
             T_iniFile iniFile,
             T_byte8 *p_catName,
             T_byte8 *p_keyName,
             T_byte8 *p_string,
             T_word16 maxLength)
{
    T_iniFileStruct *p_ini ;
    T_iniCategory *p_category ;
    T_iniKey *p_key ;
    T_byte8 *p_found = NULL ;
    T_word16 i ;
    T_doubleLinkListElement element ;
    T_word16 len ;
    E_Boolean isSpecialChar ;

    DebugRoutine("INIFileGetString") ;

    /* Return nothing at the least. */
    p_string[0] = '\0' ;
    len = 0 ;

    /* Get access to the ini data. */
    p_ini = (T_iniFileStruct *)iniFile ;
    DebugCheck(p_ini->tag == INIFILE_TAG) ;

    /* Don't bother if this is a bad ini file. */
    if (p_ini)  {
        /* First, find the category. */
        p_category = IFindCategory(p_ini, p_catName) ;
        if (p_category)  {
            /* If we found category, find the key within. */
            p_key = IFindKeyAndElement(p_category, p_keyName, &element) ;

            /* If found, return the value. */
            if (p_key)  {
                /* Append all strings under the same key name */
                while (element != DOUBLE_LINK_LIST_ELEMENT_BAD)  {
                    p_key = DoubleLinkListElementGetData(element) ;

                    /* Make sure the key name matches */
                    if(strcmp(p_key->key, p_keyName)!=0)
                        break ;

                    p_found = p_key->value ;

                    /* Parse the line into special text */
                    isSpecialChar = FALSE ;

                    /* Go through each character and parse */
                    for (i=0; p_found[i] != '\0'; i++)  {
                        /* If we have reached the limit, stop. */
                        if (len >= maxLength)
                            break ;

                        /* Underbars are always spaces */
                        if (p_found[i] == '_')
                            p_found[i] = ' ' ;

                        /* If the last character was special, interpret */
                        /* the next character */
                        if (isSpecialChar)  {
                            switch(p_found[i])  {
                                case 't':
                                    p_string[len++] = '\t' ;
                                    break ;
                                case 'r':
                                    p_string[len++] = '\r' ;
                                    break ;
                                case 'n':
                                    p_string[len++] = '\n' ;
                                    break ;
                                default:
                                    p_string[len++] = p_found[i] ;
                                    break ;
                            }
                            /* No longer special character */
                            isSpecialChar = FALSE ;
                        } else {
                            /* If not, add this character and see if */
                            /* it is a special character */
                            p_string[len] = p_found[i] ;

                            if (p_found[i] == '\\')   {
                                isSpecialChar = TRUE ;
                            } else {
                                len++ ;
                            }
                        }
                    }

                    /* Put a null character at the end */
                    p_string[len] = '\0' ;

                    /* Move to the next line in the config file */
                    element = DoubleLinkListElementGetNext(element);
                }
            }
        }
    }
    DebugEnd() ;
}

/** @} */
/*-------------------------------------------------------------------------*
 * End of File:  INIFILE.C
 *-------------------------------------------------------------------------*/
