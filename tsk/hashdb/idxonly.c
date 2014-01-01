/*
 * The Sleuth Kit
 *
 * Brian Carrier [carrier <at> sleuthkit [dot] org]
 * Copyright (c) 2007-2014 Brian Carrier.  All rights reserved
 *
 * This software is distributed under the Common Public License 1.0
 */

/**
 * \file idxonly_index.c
 * Contains the dummy functions that are used when only an index is used for lookups and the 
 * original database is gone. 
 */

#include "tsk_hashdb_i.h"
#include <assert.h>

/**
 * Set db_name using information from this database type
 *
 * @param hdb_info the hash database object
 */
void
idxonly_name(TSK_HDB_INFO * hdb_info)
{
    TSK_TEXT_HDB_INFO *text_hdb_info = (TSK_TEXT_HDB_INFO*)hdb_info;
    TSK_HDB_BINSRCH_IDX_INFO *idx_info = (TSK_HDB_BINSRCH_IDX_INFO*)text_hdb_info->idx;
    FILE * hFile;
    char buf[TSK_HDB_NAME_MAXLEN];
    char *bufptr = buf;
    size_t i = 0;
    memset(idx_info->base.db_name, '\0', TSK_HDB_NAME_MAXLEN);

    if(tsk_hdb_idxsetup(hdb_info, TSK_HDB_HTYPE_MD5_ID) == 0) {
        if (tsk_verbose)
            fprintf(stderr,
                "Failed to get name from index (index does not exist); using file name instead");
        tsk_hdb_name_from_path(hdb_info);
        return;
    }

    hFile = idx_info->hIdx;
    fseeko(hFile, 0, 0);
    if(NULL == fgets(buf, TSK_HDB_NAME_MAXLEN, hFile) ||
        NULL == fgets(buf, TSK_HDB_NAME_MAXLEN, hFile) ||
        strncmp(buf,
                TSK_HDB_IDX_HEAD_NAME_STR,
                strlen(TSK_HDB_IDX_HEAD_NAME_STR)) != 0) {
        if (tsk_verbose)
            fprintf(stderr,
                "Failed to read name from index; using file name instead");
        tsk_hdb_name_from_path(hdb_info);
        return;
    }
    bufptr = strchr(buf, '|');
    bufptr++;
    while(bufptr[i] != '\r' && bufptr[i] != '\n' && i < strlen(bufptr))
    {
        idx_info->base.db_name[i] = bufptr[i];
        i++;
    }
}

/**
 * This function creates an empty
 *
 * @param hdb_info Hash database to make index of.
 * @param dbtype Type of hash database. Ignored for IDX only.
 *
 * @return 1 on error and 0 on success.
 */
uint8_t
idxonly_makeindex(TSK_HDB_INFO * hdb_info, TSK_TCHAR * dbtype)
{
    //tsk_error_reset();
    //tsk_error_set_errno(TSK_ERR_HDB_ARG);
    //tsk_error_set_errstr(
    //         "idxonly_makeindex: Make index not supported when INDEX ONLY option is used");

    ///@temporary until we exorcise all the htype conditionals out
    TSK_TCHAR dbtype_default[1024];
    TSNPRINTF(dbtype_default, 1024, _TSK_T("%") PRIcTSK, TSK_HDB_DBTYPE_MD5SUM_STR);

    /* Initialize the TSK index file */
    if (tsk_hdb_idxinitialize(hdb_info, dbtype_default)) {
        tsk_error_set_errstr2( "idxonly_makeindex");
        return 1;
    }

    return 0;
}

/**
 * This function should find the corresponding name at a
 * given offset.  In this case though, we do not have the original database,
 * so just make an error...
 *
 * @param hdb_info Hash database to get data from
 * @param hash MD5 hash value that was searched for
 * @param offset Byte offset where hash value should be located in db_file
 * @param flags (not used)
 * @param action Callback used for each entry found in lookup
 * @param cb_ptr Pointer to data passed to callback
 *
 * @return 1 on error and 0 on succuss
 */
uint8_t
idxonly_getentry(TSK_HDB_INFO * hdb_info, const char *hash,
                 TSK_OFF_T offset, TSK_HDB_FLAG_ENUM flags,
                 TSK_HDB_LOOKUP_FN action, void *cb_ptr)
{
    tsk_error_reset();
    tsk_error_set_errno(TSK_ERR_HDB_ARG);
    tsk_error_set_errstr(
             "idxonly_getentry: Not supported when INDEX ONLY option is used");
    return 1;
}

TSK_HDB_INFO *idxonly_open(const TSK_TCHAR *idx_path)
{
    TSK_TEXT_HDB_INFO *text_hdb_info = NULL;
    size_t flen = 0;

    assert(NULL != idx_path);
    
    if ((text_hdb_info = (TSK_TEXT_HDB_INFO*)tsk_malloc(sizeof(TSK_TEXT_HDB_INFO))) == NULL) {
        return NULL;
    }

    flen = TSTRLEN(idx_path) + 8; // RJCTODO: Check this change from 32 (change was in DF code) with Brian; was change in older code? What is the point, anyway?
    text_hdb_info->base.idx_fname = (TSK_TCHAR*)tsk_malloc(flen * sizeof(TSK_TCHAR));
    if (NULL == text_hdb_info->base.idx_fname) {
        return NULL;
    }

    TSTRNCPY(text_hdb_info->base.idx_fname, idx_path, flen);
    text_hdb_info->base.db_type = TSK_HDB_DBTYPE_IDXONLY_ID;
    text_hdb_info->base.updateable = 0;
    text_hdb_info->base.uses_external_index = 1;
    text_hdb_info->base.hash_type = TSK_HDB_HTYPE_INVALID_ID; // This will be set when the index is created/opened. 
    text_hdb_info->base.hash_len = 0; // This will be set when the index is created/opened.
    tsk_init_lock(&text_hdb_info->base.lock);
    text_hdb_info->base.makeindex = idxonly_makeindex;
    text_hdb_info->base.add_comment = NULL; // RJCTODO: Consider making no-ops for these or moving them
    text_hdb_info->base.add_filename = NULL; // RJCTODO: Consider making no-ops for these or moving them

    text_hdb_info->getentry = idxonly_getentry;

    // RJCTODO: Figure out when to do this
    //idxonly_name((TSK_HDB_INFO*)hdb_info);

    return (TSK_HDB_INFO*)text_hdb_info;    
}

