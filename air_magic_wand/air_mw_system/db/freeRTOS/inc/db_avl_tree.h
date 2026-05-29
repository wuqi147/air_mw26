/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2021
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE AIROHA SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AIROHA SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT AIROHA'S OPTION, TO REVISE OR REPLACE THE AIROHA SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*******************************************************************************/
/*
* The MIT License:
*
* Copyright (C) 2016 Alexander Saprykin <saprykin.spb@gmail.com>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/* FILE NAME:  db_avl_tree.h
 * PURPOSE:
 *      DB task avl tree algorithm header file
 *
 * NOTES:
 */

#ifndef DB_AVL_TREE_H
#define DB_AVL_TREE_H


/* INCLUDE FILE DECLARATIONS
*/
#include "mw_error.h"
#include "mw_types.h"
#include "mw_utils.h"
#include "osapi_memory.h"
#include "db_notify.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
*/

/* DATA TYPE DECLARATIONS
*/
typedef struct DB_AVL_NODE_S
{
    DB_SUB_ENTITY_T          node;              /* The key and value */
    struct DB_AVL_NODE_S     *parent;           /* The parent node */
    struct DB_AVL_NODE_S     *left;             /* The left child node */
    struct DB_AVL_NODE_S     *right;            /* The right child node */
    I8_T                     bf;                /* The balance factor always should be one of the [-1, 0, 1]*/
} __attribute__((packed)) DB_AVL_NODE_T;


/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
/* FUNCTION NAME: db_find_client_head
 * PURPOSE:
 *      Find the first client of the subscription
 *
 * INPUT:
 *      root_node    --  A double pointer of the root node
 *      req          --  The new subscribed key
 *
 * OUTPUT:
 *      entity       --  The founded entity
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_find_client_head(
    DB_AVL_NODE_T **root_node,
    DB_REQUEST_TYPE_T *req,
    DB_SUB_CLIENT_T **entity);

/* FUNCTION NAME: db_find_client_next
 * PURPOSE:
 *      Find the next client of the subscription
 *
 * INPUT:
 *      entity       --  The client's entity
 *
 * OUTPUT:
 *      next         --  The next entity
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_find_client_next(
    DB_SUB_CLIENT_T *entity,
    DB_SUB_CLIENT_T **next);

/* FUNCTION NAME: db_find_client_sub
 * PURPOSE:
 *      Find the specific subscription, if exist, return the client's entity
 *
 * INPUT:
 *      root_node    --  A double pointer of the root node
 *      req          --  The new subscribed key
 *      client       --  The new subscribed value
 *
 * OUTPUT:
 *      entity       --  The founded entity of the client
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_find_client_sub(
    DB_AVL_NODE_T **root_node,
    DB_REQUEST_TYPE_T *req,
    C8_T *client,
    DB_SUB_CLIENT_T **entity);

/* FUNCTION NAME: db_find_sub_first
 * PURPOSE:
 *      Get the smallest key subsciprtion
 *
 * INPUT:
 *      root_node    --  A pointer of the root node
 *
 * OUTPUT:
 *      node         --  The node with smallest key.
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_find_sub_first(
    DB_AVL_NODE_T *root_node,
    DB_AVL_NODE_T **node);

/* FUNCTION NAME: db_find_sub_next
 * PURPOSE:
 *      Get the next subsciprtion node
 *
 * INPUT:
 *      node         --  The current node
 *
 * OUTPUT:
 *      next         --  The next node.
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_find_sub_next(
    DB_AVL_NODE_T *node,
    DB_AVL_NODE_T **next);

/* FUNCTION NAME: db_add_sub
 * PURPOSE:
 *      Add a subsciprtion into tree
 *
 * INPUT:
 *      root_node    --  A double pointer of the root node
 *      req          --  The new subscribed key
 *      client       --  The new subscribed value
 *
 * OUTPUT:
 *      root_node    --  The root node may be changed after balancing
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      Must call db_find_sub before create a new entity.
 *
 */
MW_ERROR_NO_T
db_add_sub(
    DB_AVL_NODE_T **root_node,
    DB_REQUEST_TYPE_T *req,
    C8_T *client);

/* FUNCTION NAME: db_del_sub
 * PURPOSE:
 *      remove a subsciprtion from tree
 *
 * INPUT:
 *      root_node    --  A double pointer of the root node
 *      req          --  The new subscribed key
 *      client       --  The new subscribed value
 *
 * OUTPUT:
 *      root_node    --  The root node may be changed after balancing
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      If the key has another client in list, then the key still
 *      exist, otherwise the key node will be removed from the tree.
 *
 */
MW_ERROR_NO_T
db_del_sub(
    DB_AVL_NODE_T **root_node,
    DB_REQUEST_TYPE_T *req,
    C8_T *client);


#endif /* End of DB_AVL_TREE_H */
