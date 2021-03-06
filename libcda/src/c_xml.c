#include "c_xml.h"

XML_DOC* xml_Open(XML_DOC *doc,FILE *fp,XML_MODE mode)
{
	int i_char;
	char s_char;
	int state;
	XML_NODE **node_list;
	XML_NODE *current_node;
	XML_NODE build_node;
	XML_NODE *pa_node;

	node_list=NULL;

	node_list=stack_Create(sizeof(XML_NODE*));
	if(node_list==NULL)goto fail_return;

	if(xml_Nodecreate(&build_node)==NULL)goto fail_return;
	doc->root=(XML_NODE*)malloc(sizeof(XML_NODE));
	memcpy(doc->root,&build_node,sizeof(XML_NODE));
	
	state=INSIDE_BLOCK;
	
	stack_Append(&node_list,&doc->root);
	pa_node=*stack_Get(node_list);

	for(i_char=FGETC_(fp);i_char!=EOF;i_char=FGETC_(fp))
	{
		s_char=i_char;
		switch(state)
		{
			case INSIDE_BLOCK:
			{
				if(s_char==ENCODE_('<'))state=START_HEAD;
				else if(s_char==ENCODE_('"'))
				{
					current_node=*stack_Get(node_list);
					if(current_node->data==NULL)current_node->data=string_Create();
					string_Append(&current_node->data,&s_char);
					for(s_char=FGETC_(fp);s_char!=ENCODE_('"');s_char=FGETC_(fp))
					{
						string_Append(&current_node->data,&s_char);
					};
					string_Append(&current_node->data,&s_char);
				}
				else if(s_char!=ENCODE_('\n')||s_char!=ENCODE_('\t'))
				{
					current_node=*stack_Get(node_list);
					if(current_node->data==NULL)current_node->data=string_Create();
					string_Append(&current_node->data,&s_char);
					if(current_node->data==NULL)goto fail_return;
				}
				else
				{};
				break;
			}
			case START_HEAD:
			{
				current_node=*stack_Get(node_list);
				if(s_char==ENCODE_('/'))
				{
					while(s_char!=ENCODE_('>'))s_char=FGETC_(fp);
					stack_Remove(&node_list);
					pa_node=*stack_Get(node_list);
					state=INSIDE_BLOCK;
				}
				else if(s_char==ENCODE_('!'))
				{
					while(s_char!=ENCODE_('>'))s_char=FGETC_(fp);
					s_char=FGETC_(fp);
					state=INSIDE_BLOCK;
				}
				else
				{
					if(xml_Nodecreate(&build_node)==NULL)goto fail_return;

					if(pa_node->childs==NULL)pa_node->childs=array_Create(sizeof(XML_NODE));
					if(pa_node->childs==NULL)goto fail_return;
					array_Append(&pa_node->childs,&build_node);
					current_node=&(pa_node->childs[array_Head(pa_node->childs)->array_length-1]);

					stack_Append(&node_list,&current_node);
					pa_node=*stack_Get(node_list);
					current_node=pa_node;
					if(current_node->name==NULL)current_node->name=array_Create(sizeof(CHAR_));
					if(current_node->name==NULL)goto fail_return;
					string_Append(&current_node->name,&s_char);
					state=INSIDE_HEAD;
				};
				break;
			}
			case INSIDE_HEAD:
			{
				if(s_char==ENCODE_('/'))
				{
					while(s_char!=ENCODE_('>'))s_char=FGETC_(fp);

					stack_Remove(&node_list);
					pa_node=*stack_Get(node_list);
					state=INSIDE_BLOCK;
				}
				else if(s_char==ENCODE_('>'))
				{
					current_node=*stack_Get(node_list);
					if(current_node->name!=NULL)
					{
						s_char=ENCODE_('\0');
						string_Append(&current_node->name,&s_char);
					};
					state=INSIDE_BLOCK;
				}
				else if(s_char==ENCODE_('"'))
				{
					current_node=*stack_Get(node_list);
					string_Append(&current_node->name,&s_char);
					for(s_char=FGETC_(fp);s_char!=ENCODE_('"');s_char=FGETC_(fp))
					{
						string_Append(&current_node->name,&s_char);
					};
					string_Append(&current_node->name,&s_char);
				}
				else
				{
					current_node=*stack_Get(node_list);
					string_Append(&current_node->name,&s_char);
					state=INSIDE_HEAD;
				};
				break;
			};
			default:
			{
				goto fail_return;
			};/*nothing to do*/
		};
	};

	if(array_Head(node_list)->array_length>1)goto fail_return;
	else
	{
		stack_Drop(&node_list);
	};

	return doc;

	fail_return:
	stack_Drop(&node_list);
	return NULL;
};

C_STRING xml_Storedata(C_STRING *str,XML_NODE const *node,XML_DOC const *doc)
{
	assert(str!=NULL&&node!=NULL&&doc!=NULL);
	if(str!=NULL&&node!=NULL&&doc!=NULL)
	{
		string_Set(str,node->data);
		return *str;
	}
	return NULL;
};

XML_NODE* xml_Nodecreate(XML_NODE *node)
{
	assert(node!=NULL);
	if(node!=NULL)
	{
		node->name=NULL;
		node->data=NULL;
		node->childs=NULL;
		return node;
	}
	return NULL;
};

XML_NODE* xml_Nodebyname(CHAR_ const *name,XML_NODE const *source)
{
	size_t i;
	C_STRING cache;

	assert(name!=NULL&&source!=NULL);
	if(name!=NULL&&source!=NULL)
	{
		cache=string_Create();
		if(source->childs!=NULL)for(i=0;i<xml_Nodechilds(source);++i)if(!STRCMP_(xml_Nodename(&cache,&source->childs[i]),name))
		{
			string_Drop(&cache);
			return &source->childs[i];
		};
		string_Drop(&cache);
		return NULL;
	}
	return NULL;
};

C_STRING xml_Parmbyname(C_STRING *parm_data,CHAR_ const *name,XML_NODE const *source)
{
	CHAR_ format[]=" %s=";
	C_STRING ca_string=NULL;
	size_t ca_length;
	KMP_KEY name_key;
	CHAR_ *loc;
	BOOL_ inside_str;

	assert(parm_data!=NULL&&name!=NULL&&source!=NULL);
	if(parm_data!=NULL&&name!=NULL&&source!=NULL)
	{
		ca_length=STRLEN_(format)+STRLEN_(name);
		ca_string=string_Create();
		array_Resize(&ca_string,ca_length);

		SNPRINTF_(ca_string,ca_length,format,name);

		if(string_Kprepare(&name_key,ca_string)==NULL)goto fail_return;
		loc=string_Knsearch(source->name,&name_key,STRLEN_(source->name));
		if(loc==NULL)goto fail_return;
		array_Resize(parm_data,0);
		inside_str=FALSE_;
		for(loc+=ca_length-2;!(inside_str==FALSE_&&*loc==ENCODE_(' '))&&(*loc!=ENCODE_('\0'))&&(*loc!=ENCODE_('>'));++loc)
		{
			if(*loc==ENCODE_('"')&&inside_str==FALSE_)inside_str=TRUE_;
			else if(*loc==ENCODE_('"')&&inside_str==TRUE_)inside_str=FALSE_;
			string_Append(parm_data,loc);
		};

		string_Kfree(&name_key);
		string_Drop(&ca_string);
		return *parm_data;
	}

	fail_return:
	if(ca_string!=NULL)string_Drop(&ca_string);
	return NULL;
};

C_STRING xml_Parmremove(XML_NODE const *source,CHAR_ const *name)
{
	CHAR_ format[]=" %s=";

	C_STRING ca_string=NULL;
	size_t ca_length;
	KMP_KEY name_key;
	CHAR_ *loc;
	BOOL_ inside_str;
	CHAR_ *start;

	assert(source!=NULL&&name!=NULL);
	if(source!=NULL&&name!=NULL)
	{
		ca_length=STRLEN_(format)+STRLEN_(name);
		ca_string=string_Create();
		array_Resize(&ca_string,ca_length);

		SNPRINTF_(ca_string,ca_length,format,name);

		if(string_Kprepare(&name_key,ca_string)==NULL)goto fail_return;
		loc=string_Knsearch(source->name,&name_key,STRLEN_(source->name));
		if(loc==NULL)goto fail_return;
		start=loc;
		inside_str=FALSE_;
		for(loc+=ca_length-2;!(inside_str==FALSE_&&*loc==ENCODE_(' '))&&(*loc!=ENCODE_('\0'))&&(*loc!=ENCODE_('>'));++loc)
		{
			if(*loc==ENCODE_('"')&&inside_str==TRUE_)inside_str=FALSE_;
			else if(*loc==ENCODE_('"'))inside_str=TRUE_;
		};

		memmove(start,loc,(STRLEN_(loc)+1)*sizeof(CHAR_));

		string_Kfree(&name_key);
		string_Drop(&ca_string);
		return source->name;
	}

	fail_return:
	if(ca_string!=NULL)string_Drop(&ca_string);
	string_Kfree(&name_key);
	return NULL;
};

C_STRING xml_Parmset(XML_NODE *dest,CHAR_ const *name,CHAR_ const *data)
{
	assert(dest!=NULL&&name!=NULL&&data!=NULL);
	if(dest!=NULL&&name!=NULL&&data!=NULL)
	{
		xml_Parmremove(dest,name);
		xml_Parmappend(dest,name,data);
	}
	return NULL;
};

C_STRING xml_Nodename(C_STRING *node_name,XML_NODE const *node)
{
	CHAR_ *loc;

	assert(node_name!=NULL&&node!=NULL);
	if(node_name!=NULL&&node!=NULL)
	{
		array_Resize(node_name,0);
		for(loc=node->name;*loc!=ENCODE_(' ')&&*loc!=ENCODE_('\0');++loc)string_Append(node_name,loc);
		return *node_name;
	}
	return NULL;
};

XML_NODE* xml_Nodeappend(XML_NODE *dest,XML_NODE const *source)
{
	assert(dest!=NULL&&source!=NULL);
	if(dest!=NULL&&source!=NULL)
	{
		if(dest->childs==NULL)dest->childs=array_Create(sizeof(XML_NODE));
		if(dest->childs==NULL)goto fail_return;
		array_Append(&dest->childs,source);
		return dest;
	}

	fail_return:
	return NULL;
};

XML_NODE* xml_Parmappend(XML_NODE *dest,CHAR_ const *name,CHAR_ const *data)
{
	C_STRING new_parm;
	CHAR_ format[]=ENCODE_(" %s=%s");
	CHAR_ *op;

	assert(dest!=NULL&&name!=NULL&&data!=NULL);
	new_parm=array_Create(sizeof(CHAR_));
	array_Resize(&new_parm,STRLEN_(name)+STRLEN_(data)+STRLEN_(format));
	SNPRINTF_(new_parm,STRLEN_(name)+STRLEN_(data)+STRLEN_(format),format,name,data);
	array_Resize(&dest->name,STRLEN_(dest->name));
	for(op=new_parm;*op!=ENCODE_('\0');++op)string_Append(&dest->name,op);
	string_Drop(&new_parm);
	return dest;
};

XML_NODE* xml_Noderemove(XML_NODE *dest,XML_NODE *tag)
{
	assert(dest!=NULL&&tag!=NULL);
	array_Remove(&dest->childs,tag);
	return dest;
};

BOOL_ xml_Save(FILE *file,XML_DOC const *doc)
{
	assert(file!=NULL&&doc!=NULL);
	SAVE_NODE *node_list;
	SAVE_NODE *current_node;
	SAVE_NODE add_node;
	XML_NODE *current_child;
	C_STRING name_cache;

	node_list=stack_Create(sizeof(SAVE_NODE));
	name_cache=string_Create();
	add_node.node=doc->root;
	add_node.n=0;
	stack_Append(&node_list,&add_node);
	do
	{
		current_node=stack_Get(node_list);
		current_child=&(current_node->node->childs[current_node->n]);
		FPRINTF_(file,XML_HEAD,current_child->name);
		
		if(current_child->childs!=NULL)
		{
			add_node.node=current_child;
			add_node.n=0;
			stack_Append(&node_list,&add_node);
		}
		else
		{
			if((current_node->n)>=(array_Length(current_node->node->childs)-1))
			{
				stack_Remove(&node_list);
				if(current_child->data!=NULL)FPRINTF_(file,XML_DATA,current_child->data);
				FPRINTF_(file,XML_TAIL,xml_Nodename(&name_cache,current_child));
				FPRINTF_(file,XML_TAIL,xml_Nodename(&name_cache,current_node->node));
			}
			else
			{
				if(current_child->data!=NULL)FPRINTF_(file,XML_DATA,current_child->data);
				FPRINTF_(file,XML_TAIL,xml_Nodename(&name_cache,current_child));
				++current_node->n;
			};
		};
		
	}while(array_Length(node_list)>1);

	stack_Drop(&node_list);
	string_Drop(&name_cache);
	return TRUE_;

	/*fail_return:*/
	/*return FALSE_;*/
};

void xml_Nodefree(XML_NODE *node)
{
	XML_NODE *node_list;
	XML_NODE current_node;
	size_t n;

	assert(node!=NULL);
	node_list=stack_Create(sizeof(XML_NODE));

	stack_Append(&node_list,node);

	while(array_Length(node_list)>0)
	{
		current_node=*stack_Get(node_list);
		stack_Remove(&node_list);
		if(current_node.name!=NULL)string_Drop(&current_node.name);
		if(current_node.data!=NULL)string_Drop(&current_node.data);
		if(current_node.childs!=NULL)
		{
			for(n=0;n<array_Length(current_node.childs);++n)
			{
				stack_Append(&node_list,&current_node.childs[n]);
			};
			array_Drop(&current_node.childs);
		};
	};
	stack_Drop(&node_list);
};

void xml_Close(XML_DOC *doc)
{
	assert(doc!=NULL);
	xml_Nodefree(doc->root);
	free(doc->root);
};
