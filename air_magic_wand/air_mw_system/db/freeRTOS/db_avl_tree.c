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
/* FILE NAME:  db_avl_tree.c
 * PURPOSE:
 *      DB task avl tree algorithm header file
 *
 * NOTES:
 *      Refer to saprykin/plibsys src/ptree-avl.c with MIT license
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "db_avl_tree.h"
#include "db_main.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void _db_sub_tree_rotate_left(DB_AVL_NODE_T **p_root, DB_AVL_NODE_T* p_sub);
static void _db_sub_tree_rotate_right(DB_AVL_NODE_T **p_root, DB_AVL_NODE_T* p_sub);
static void _db_sub_tree_rotate_left_right(DB_AVL_NODE_T **p_root, DB_AVL_NODE_T* p_sub);
static void _db_sub_tree_rotate_right_left(DB_AVL_NODE_T **p_root, DB_AVL_NODE_T* p_sub);
static void _db_sub_tree_balance_insert(DB_AVL_NODE_T **p_root, DB_AVL_NODE_T *p_sub);
static void _db_sub_tree_balance_remove(DB_AVL_NODE_T **p_root, DB_AVL_NODE_T *p_sub);

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROMGRAM BODIES
*/
static void
_db_sub_tree_rotate_left(
    DB_AVL_NODE_T **p_root,
    DB_AVL_NODE_T *p_sub)
{
    if ((NULL == p_root) || (NULL == p_sub))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return;
    }

    p_sub->parent->right = p_sub->left;
    if (NULL != p_sub->left)
    {
        p_sub->left->parent = p_sub->parent;
    }

    p_sub->left = p_sub->parent;
    p_sub->parent = p_sub->left->parent;
    p_sub->left->parent = p_sub;

    if (NULL != p_sub->parent)
    {
        if (p_sub->parent->left == p_sub->left)
        {
            p_sub->parent->left = p_sub;
        }
        else
        {
            p_sub->parent->right= p_sub;
        }
    }
    else
    {
        *p_root = p_sub;
    }

    /* Restore the balance factor */
    p_sub->bf += 1;
    p_sub->left->bf = -p_sub->bf;
    DB_LOG_DEBUG("rotate left, p_parent(%p), left(%p)<-p_sub(%p)->right(%p)",p_sub->parent, p_sub->left, p_sub, p_sub->right);
}

static void
_db_sub_tree_rotate_right(
    DB_AVL_NODE_T **p_root,
    DB_AVL_NODE_T* p_sub)
{
    if ((NULL == p_root) || (NULL == p_sub))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return;
    }

    p_sub->parent->left = p_sub->right;
    if (NULL != p_sub->right)
    {
        p_sub->right->parent = p_sub->parent;
    }

    p_sub->right= p_sub->parent;
    p_sub->parent = p_sub->right->parent;
    p_sub->right->parent = p_sub;

    if (NULL != p_sub->parent)
    {
        if (p_sub->parent->left == p_sub->right)
        {
            p_sub->parent->left = p_sub;
        }
        else
        {
            p_sub->parent->right= p_sub;
        }
    }
    else
    {
        *p_root = p_sub;
    }

    /* Restore the balance factor */
    p_sub->bf -= 1;
    p_sub->right->bf = -p_sub->bf;
    DB_LOG_DEBUG("rotate right, p_parent(%p), left(%p)<-p_sub(%p)->right(%p)",p_sub->parent, p_sub->left, p_sub, p_sub->right);
}

static void
_db_sub_tree_rotate_left_right(
    DB_AVL_NODE_T **p_root,
    DB_AVL_NODE_T *p_sub)
{
    DB_AVL_NODE_T *tmp = NULL;

    if ((NULL == p_root) || (NULL == p_sub))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return;
    }

    tmp = p_sub->right;
    p_sub->right = tmp->left;

    if (NULL != p_sub->right)
    {
        p_sub->right->parent = p_sub;
    }

    tmp->parent = p_sub->parent->parent;

    if (NULL != tmp->parent)
    {
        if (tmp->parent->left == p_sub->parent)
        {
            tmp->parent->left = tmp;
        }
        else
        {
            tmp->parent->right= tmp;
        }
    }
    else
    {
        *p_root = tmp;
    }

    p_sub->parent->left = tmp->right;

    if (NULL != p_sub->parent->left)
    {
        p_sub->parent->left->parent = p_sub->parent;
    }

    tmp->right = p_sub->parent;
    tmp->right->parent = tmp;

    tmp->left = p_sub;
    p_sub->parent = tmp;

    /* Restore the balance factor */
    if (1 == tmp->bf)
    {
        tmp->left->bf = 0;
        tmp->right->bf = -1;
    }
    else if (-1 == tmp->bf)
    {
        tmp->left->bf = 1;
        tmp->right->bf = 0;
    }
    else
    {
        tmp->left->bf = 0;
        tmp->right->bf = 0;
    }
    tmp->bf = 0;
    DB_LOG_DEBUG("rotate left right, psub p_parent(%p), left(%p)<-p_sub(%p)->right(%p)",p_sub->parent, p_sub->left, p_sub, p_sub->right);
    DB_LOG_DEBUG("rotate left right, tmp p_parent(%p), left(%p)<-tmp(%p)->right(%p)",tmp->parent, tmp->left, tmp, tmp->right);
}

static void
_db_sub_tree_rotate_right_left(
    DB_AVL_NODE_T **p_root,
    DB_AVL_NODE_T* p_sub)
 {
    DB_AVL_NODE_T *tmp;

    if ((NULL == p_root) || (NULL == p_sub))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return;
    }

    tmp = p_sub->left;
    p_sub->left = tmp->right;

    if (NULL != p_sub->left)
    {
        p_sub->left->parent = p_sub;
    }

    tmp->parent = p_sub->parent->parent;

    if (NULL != tmp->parent)
    {
        if (tmp->parent->left == p_sub->parent)
        {
            tmp->parent->left = tmp;
        }
        else
        {
            tmp->parent->right= tmp;
        }
    }
    else
    {
        *p_root = tmp;
    }

    p_sub->parent->right = tmp->left;

    if (NULL != p_sub->parent->right)
    {
        p_sub->parent->right->parent = p_sub->parent;
    }

    tmp->left = p_sub->parent;
    tmp->left->parent = tmp;

    tmp->right = p_sub;
    p_sub->parent = tmp;

    /* Restore the balance factor */
    if (1 == tmp->bf)
    {
        tmp->left->bf = 0;
        tmp->right->bf = -1;
    }
    else if (-1 == tmp->bf)
    {
        tmp->left->bf = 1;
        tmp->right->bf = 0;
    }
    else
    {
        tmp->left->bf = 0;
        tmp->right->bf = 0;
    }
    tmp->bf = 0;
    DB_LOG_DEBUG("rotate right left, psub p_parent(%p), left(%p)<-p_sub(%p)->right(%p)",p_sub->parent, p_sub->left, p_sub, p_sub->right);
    DB_LOG_DEBUG("rotate right left, tmp p_parent(%p), left(%p)<-tmp(%p)->right(%p)",tmp->parent, tmp->left, tmp, tmp->right);
}

static void
_db_sub_tree_balance_insert(
    DB_AVL_NODE_T **p_root,
    DB_AVL_NODE_T *p_sub)
{
    DB_AVL_NODE_T *p_parent = NULL;

    if ((NULL == p_root) || (NULL == p_sub))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return;
    }

    while (TRUE)
    {
        p_parent = p_sub->parent;

        if (NULL == p_parent)
        {
            /* root node */
            break;
        }

        if (p_parent->left == p_sub)
        {
            if (p_parent->bf == 1)
            {
                if (p_sub->bf == -1)
                {
                    _db_sub_tree_rotate_left_right(p_root, p_sub);
                }
                else
                {
                    _db_sub_tree_rotate_right(p_root, p_sub);
                }
                break;
            }
            else if (p_parent->bf == -1)
            {
                p_parent->bf = 0;
                break;
            }
            else
            {
                p_parent->bf = 1;
            }
        }
        else
        {
            if (p_parent->bf == -1)
            {
                if (p_sub->bf == 1)
                {
                    _db_sub_tree_rotate_right_left(p_root, p_sub);
                }
                else
                {
                    _db_sub_tree_rotate_left(p_root, p_sub);
                }
                break;
            }
            else if (p_parent->bf == 1)
            {
                p_parent->bf = 0;
                break;
            }
            else
            {
                p_parent->bf = -1;
            }
        }
        p_sub = p_sub->parent;
    }
}

static void
_db_sub_tree_balance_remove(
    DB_AVL_NODE_T **p_root,
    DB_AVL_NODE_T *p_sub)
{
    DB_AVL_NODE_T *p_parent = NULL;
    DB_AVL_NODE_T *p_sibling = NULL;
    I8_T sibling_bf = 0;

    if ((NULL == p_root) || (NULL == p_sub))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return;
    }

    while (TRUE)
    {
        p_parent = p_sub->parent;

        if (NULL == p_parent)
        {
            /* root node */
            break;
        }

        if (p_parent->left == p_sub)
        {
            if (p_parent->bf == -1)
            {
                p_sibling = p_parent->right;
                sibling_bf = p_sibling->bf;
                if (p_sibling->bf == 1)
                {
                    _db_sub_tree_rotate_right_left(p_root, p_sibling);
                }
                else
                {
                    _db_sub_tree_rotate_left(p_root, p_sibling);
                }
                p_sub = p_parent;

                if (0 == sibling_bf)
                {
                    break;
                }
            }
            else if (p_parent->bf == 0)
            {
                p_parent->bf = -1;
                break;
            }
            else
            {
                p_parent->bf = 0;
            }
        }
        else
        {
            if (p_parent->bf == 1)
            {
                p_sibling = p_parent->left;
                sibling_bf = p_sibling->bf;
                if (p_sibling->bf == -1)
                {
                    _db_sub_tree_rotate_left_right(p_root, p_sibling);
                }
                else
                {
                    _db_sub_tree_rotate_right(p_root, p_sibling);
                }
                p_sub = p_parent;

                if (0 == sibling_bf)
                {
                    break;
                }
            }
            else if (p_parent->bf == 0)
            {
                p_parent->bf = 1;
                break;
            }
            else
            {
                p_parent->bf = 0;
            }
        }
        p_sub = p_sub->parent;
    }

}

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
    DB_SUB_CLIENT_T **entity)
{
    MW_ERROR_NO_T ret = MW_E_ENTRY_NOT_FOUND;
    DB_AVL_NODE_T *p_sub = NULL;
    I32_T cmp = 0;


    if ((NULL == root_node) || (NULL == req) || (NULL == entity))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return MW_E_OTHERS;
    }
    if (NULL == *root_node)
    {
        DB_LOG_DEBUG("%s", "The subscription tree is empty");
        return ret;
    }

    /* search the subscribed table and field */
    p_sub = *root_node;
    while (NULL != p_sub)
    {
        cmp = memcmp((const void *)(&(p_sub->node.key)),
                     (const void *)req,
                     sizeof(DB_REQUEST_TYPE_T));
        if (cmp < 0)
        {
            /* p_sub less than request */
            p_sub = p_sub->right;
            continue;
        }
        if (cmp > 0)
        {
            /* request less than p_sub */
            p_sub = p_sub->left;
            continue;
        }
        /* request equals to p_sub
         * search the subscribed client and index
         */
        *entity = p_sub->node.c_head;
        ret = MW_E_OK;
        break;
    }

    return ret;
}

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
    DB_SUB_CLIENT_T **next)
{
    MW_ERROR_NO_T ret = MW_E_ENTRY_NOT_FOUND;

    if ((NULL == entity) || (NULL == next))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return MW_E_OTHERS;
    }

    *next = entity->c_next;
    if (NULL != *next)
    {
        ret = MW_E_OK;
    }
    return ret;
}

/* FUNCTION NAME: db_find_sub
 * PURPOSE:
 *      Find the specific subscription, if exist, return the client's entity
 *
 * INPUT:
 *      root_node    --  A double pointer of the root node
 *      req          --  The new subscribed key
 *      client       --  The new subscribed value
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
db_find_client_sub(
    DB_AVL_NODE_T **root_node,
    DB_REQUEST_TYPE_T *req,
    C8_T *client,
    DB_SUB_CLIENT_T **entity)
{
    MW_ERROR_NO_T ret = MW_E_ENTRY_NOT_FOUND;
    DB_AVL_NODE_T *p_sub = NULL;
    I32_T cmp = 0;

    if ((NULL == root_node) || (NULL == req) || (NULL == client) || (NULL == entity))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return MW_E_OTHERS;
    }
    if (NULL == *root_node)
    {
        DB_LOG_DEBUG("%s", "The subscription tree is empty");
        return ret;
    }

    /* search the subscribed table and field */
    p_sub = *root_node;
    while (NULL != p_sub)
    {
        cmp = memcmp((const void *)(&(p_sub->node.key)),
                     (const void *)req,
                     sizeof(DB_REQUEST_TYPE_T));
        if (cmp < 0)
        {
            /* p_sub less than request */
            p_sub = p_sub->right;
            continue;
        }
        if (cmp > 0)
        {
            /* request less than p_sub */
            p_sub = p_sub->left;
            continue;
        }
        /* request equals to p_sub
         * search the subscribed client and index
         */
        for (*entity = p_sub->node.c_head; *entity; *entity = (*entity)->c_next)
        {
            if (0 == osapi_strncmp((*entity)->cq_name, client, DB_Q_NAME_SIZE))
            {
                /* founded, break the loop */
                ret = MW_E_OK;
                break;
            }
        }
        break;
    }

    return ret;
}

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
    DB_AVL_NODE_T **node)
{
    MW_ERROR_NO_T ret = MW_E_ENTRY_NOT_FOUND;
    DB_AVL_NODE_T *p_node = NULL;

    if ((NULL == root_node) || (NULL == node))
    {
        return ret;
    }

    p_node = root_node;
    while (NULL != p_node->left)
    {
        p_node = p_node->left;
    }

    if (NULL != p_node)
    {
        *node = p_node;
        ret = MW_E_OK;
    }
    return ret;
}

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
    DB_AVL_NODE_T **next)
{
    MW_ERROR_NO_T ret = MW_E_ENTRY_NOT_FOUND;
    DB_AVL_NODE_T *p_node = NULL;
    DB_AVL_NODE_T *parent = NULL;
    DB_AVL_NODE_T *p_next = NULL;

    if ((NULL == node) || (NULL == next))
    {
        return ret;
    }

    p_node = node;
    if (NULL != p_node->right)
    {
        p_next = p_node->right;
        while (NULL != p_next->left)
        {
            p_next = p_next->left;
        }
    }

    if (NULL == p_next)
    {
        parent = p_node->parent;
        while (p_node == parent->right)
        {
            p_node = parent;
            parent = p_node->parent;

        }
        if (p_node == parent->left)
        {
            p_next = parent;
        }
    }

    if (NULL != p_next)
    {
        *next = p_next;
        ret = MW_E_OK;
    }

    return ret;
}


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
    C8_T *client)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    I32_T cmp = 0;
    DB_AVL_NODE_T **p_sub = NULL;
    DB_AVL_NODE_T *p_parent = NULL;
    DB_SUB_ENTITY_T *p_new = NULL;
    DB_SUB_CLIENT_T *new_client = NULL;
    DB_SUB_CLIENT_T *p_client = NULL;


    if ((NULL == root_node) || (NULL == req) || (NULL == client))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return MW_E_OTHERS;
    }

    /* search the subscribed table and field */
    DB_LOG_DEBUG("root subscription node : 0x%p", *root_node);
    p_sub = root_node;
    while (NULL != *p_sub)
    {
        DB_LOG_DEBUG("p_sub node: 0x%p, left: 0x%p, right: 0x%p, parent: 0x%p",
                (*p_sub), (*p_sub)->left, (*p_sub)->right, (*p_sub)->parent);
        cmp = memcmp((const void *)(&((*p_sub)->node.key)),
                     (const void *)req,
                     sizeof(DB_REQUEST_TYPE_T));
        if (cmp < 0)
        {
            /* p_sub less than request */
            p_parent = *p_sub;
            p_sub = &((*p_sub)->right);
        }
        else if (cmp > 0)
        {
            /* request less than p_sub */
            p_parent = *p_sub;
            p_sub = &((*p_sub)->left);
        }
        else
        {
            break;
        }
    }

    /* The key entity does not exist, create one */
    if (NULL == *p_sub)
    {
        ret = db_calloc(sizeof(DB_AVL_NODE_T), (void **)p_sub);
        if (MW_E_OK != ret)
        {
            return ret;
        }
        p_new = &((*p_sub)->node);
        memcpy((void *)&(p_new->key), (const void *)req, sizeof(DB_REQUEST_TYPE_T));
        p_new->c_head = NULL;
        DB_LOG_DEBUG("create a new sub node (%p) [%d/%d/%d]", *p_sub, p_new->key.t_idx, p_new->key.f_idx, p_new->key.e_idx);
        (*p_sub)->parent = p_parent;
        (*p_sub)->left = NULL;
        (*p_sub)->right = NULL;
        (*p_sub)->bf = 0;
        DB_LOG_DEBUG("p_sub node: 0x%p, left: 0x%p, right: 0x%p, parent: 0x%p",
                (*p_sub), (*p_sub)->left, (*p_sub)->right, (*p_sub)->parent);
    }

    /* Create a new client */
    ret = db_calloc(sizeof(DB_SUB_CLIENT_T), (void **)&new_client);
    if (MW_E_OK != ret)
    {
        if (NULL != p_new)
        {
            osapi_free(*p_sub);
        }
        return ret;
    }
    osapi_strncpy(new_client->cq_name, client, DB_Q_NAME_SIZE);
    new_client->cq_name[DB_Q_NAME_SIZE - 1] = '\0';
    new_client->c_next = NULL;

    /* Add to client list */
    p_client = (*p_sub)->node.c_head;
    if (NULL == p_client)
    {
        /* the first client */
        (*p_sub)->node.c_head = new_client;
    }
    else
    {
        /* the last client */
        while (NULL != p_client)
        {
            if (NULL == p_client->c_next)
            {
                p_client->c_next = new_client;
                break;
            }
            p_client = p_client->c_next;
        }
    }

    DB_LOG_DEBUG("create a new sub client (%p) [%s]", new_client, new_client->cq_name);
    /* need to balance tree */
    if (NULL != p_new)
    {
        _db_sub_tree_balance_insert(root_node, *p_sub);
    }
    /* else did not change the tree */

    return ret;
}

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
    C8_T *client)
{
    MW_ERROR_NO_T ret = MW_E_ENTRY_NOT_FOUND;
    I32_T cmp = 0;
    DB_AVL_NODE_T *p_sub = NULL;
    DB_AVL_NODE_T *p_prev = NULL;
    DB_AVL_NODE_T *p_child = NULL;
    DB_AVL_NODE_T *p_childParent = NULL;
    DB_SUB_CLIENT_T *p_client = NULL;
    DB_SUB_CLIENT_T *p_cprev = NULL;


    if ((NULL == root_node) || (NULL == req) || (NULL == client))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return MW_E_OTHERS;
    }
    if (NULL == *root_node)
    {
        DB_LOG_DEBUG("%s", "The subscription tree is empty");
        return ret;
    }

    /* search the subscribed table and field */
    DB_LOG_DEBUG("root subscription node : 0x%p", *root_node);
    p_sub = *root_node;
    while (NULL != p_sub)
    {
        DB_LOG_DEBUG("p_sub node: 0x%p, left: 0x%p, right: 0x%p, parent: 0x%p",
                p_sub, p_sub->left, p_sub->right, p_sub->parent);
        cmp = memcmp((const void *)(&(p_sub->node.key)),
                     (const void *)req,
                     sizeof(DB_REQUEST_TYPE_T));
        if (cmp < 0)
        {
            /* p_sub less than request */
            p_sub = p_sub->right;
            continue;
        }
        if (cmp > 0)
        {
            /* request less than p_sub */
            p_sub = p_sub->left;
            continue;
        }
        /* request equals to p_sub
         * search the subscribed client and index
         */
        p_cprev = p_sub->node.c_head;
        p_client = p_cprev;
        while (NULL != p_client)
        {
            if (0 == osapi_strncmp(p_client->cq_name, client, DB_Q_NAME_SIZE))
            {
                /* founded, delete the client */
                ret = MW_E_OK;
                if (p_cprev != p_client)
                {
                    /* another client exist, return directly */
                    p_cprev->c_next = p_client->c_next;
                    osapi_free(p_client);
                    break;
                }
                else if (NULL != p_client->c_next)
                {
                    /* First node and another client exist. */
                    p_sub->node.c_head = p_client->c_next;
                    osapi_free(p_client);
                    break;
                }
                osapi_free(p_client);
                p_sub->node.c_head = NULL;
                /* remove the subscription entity */
                if ((NULL != p_sub->left) && (NULL != p_sub))
                {
                    p_prev = p_sub->left;
                    while (NULL != p_prev->right)
                    {
                         p_prev = p_prev->right;
                    }
                    memcpy((void *)&(p_sub->node.key), (const void *)&(p_prev->node.key), sizeof(DB_REQUEST_TYPE_T));
                    p_sub->node.c_head = p_prev->node.c_head;
                    p_sub = p_prev;
                }

                p_child = (NULL == p_sub->left) ? p_sub->right : p_sub->left;
                if (NULL == p_child)
                {
                    _db_sub_tree_balance_remove(root_node, p_sub);
                }

                /* Replace node with its child */
                if (p_sub == *root_node)
                {
                    *root_node = p_child;
                    p_childParent = NULL;
                }
                else
                {
                    p_childParent = p_sub->parent;

                    if (p_sub == p_childParent->left)
                    {
                        p_childParent->left = p_child;
                    }
                    else
                    {
                        p_childParent->right = p_child;
                    }
                }

                if (NULL != p_child)
                {
                    p_child->parent = p_childParent;
                    /* Balance the tree */
                    _db_sub_tree_balance_remove(root_node, p_child);
                }

                /* Free unused node */
                osapi_free(p_sub);
                break;
            }
            p_cprev = p_client;
            p_client = p_cprev->c_next;
        }
        break;
    }


    return ret;
}

