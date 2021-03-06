#ifndef XMLO_H
#define XMLO_H
#include <stdio.h>

#include "c_string.h"
#include "c_stack.h"

#define XML_HEAD ENCODE_("<%s>")
#define XML_TAIL ENCODE_("</%s>")
#define XML_DATA ENCODE_("%s")

typedef enum xml_mode
{
	STORE_ALL=0,
	STORE_INDEX=1,
	STORE_NONE=2
}XML_MODE;

typedef enum xml_state
{
	START_HEAD=0,
	INSIDE_HEAD=1,
	INSIDE_BLOCK=2
}XML_STATE;

typedef struct xml_node
{
	C_STRING name;
	C_STRING data;
	struct xml_node *childs;
}XML_NODE;

typedef struct xml_doc
{
	XML_NODE *root;
}XML_DOC;

typedef struct save_node
{
	XML_NODE *node;
	size_t n;
}SAVE_NODE;

XML_DOC* xml_Open(XML_DOC *doc,FILE *fp,XML_MODE mode);

C_STRING xml_Storedata(C_STRING *str,XML_NODE const *node,XML_DOC const *doc);

XML_NODE* xml_Nodecreate(XML_NODE *node);

#define xml_Nodechilds(source) (source->childs==NULL?0:array_Length(source->childs))

#define xml_Nodebyindex(n,source) ((source->childs==NULL)?NULL:(n<array_Head(source->childs)->array_length?&(source->childs[n]):NULL))

XML_NODE* xml_Nodebyname(CHAR_ const *name,XML_NODE const *source);

C_STRING xml_Parmbyname(C_STRING *parm_data,CHAR_ const *name,XML_NODE const *source);

C_STRING xml_Parmremove(XML_NODE const *source,CHAR_ const *name);

C_STRING xml_Parmset(XML_NODE *dest,CHAR_ const *name,CHAR_ const *data);

C_STRING xml_Nodename(C_STRING *node_name,XML_NODE const *node);

XML_NODE* xml_Nodeappend(XML_NODE *dest,XML_NODE const *source);

XML_NODE* xml_Parmappend(XML_NODE *dest,CHAR_ const *name,CHAR_ const *data);

XML_NODE* xml_Noderemove(XML_NODE *dest,XML_NODE *tag);

BOOL_ xml_Save(FILE *file,XML_DOC const *doc);

void xml_Nodefree(XML_NODE *node);

void xml_Close(XML_DOC *doc);
#endif
