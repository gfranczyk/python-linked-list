#include "llist.h"

static PyObject* TestError;
static PyObject* call_back;

static PyObject * test_set_call_back(PyObject *self, PyObject *args) {
  PyObject *temp;

  if (PyArg_ParseTuple(args, "O:set_callback", &temp)) {
      if (!PyCallable_Check(temp)) {
          PyErr_SetString(PyExc_TypeError,"Parameter is not callable");
          return NULL;
      }
      Py_XINCREF(temp);
      Py_XDECREF(call_back);
      call_back = temp;
      Py_INCREF(Py_None);
      return Py_None;
  }
  return NULL;
}

static PyObject * test_firstfnc(PyObject *self, PyObject *args) {

  char *input;
  const char *direction;

  if (!PyArg_ParseTuple(args, "ss", &input, &direction))
    return NULL;
  if (*direction == 's')
    return Py_BuildValue("s",input);
  else if (*direction == 'u') {
      PyObject *arg_list = Py_BuildValue("(s)",input);
      PyObject *result;
      result = PyObject_CallObject(call_back, arg_list);
      Py_DECREF(arg_list);
      return result;
  }
  int len = strlen(input);
  for(int i = 0; i < len/2; i++) {
      char tmp;
      tmp = *(input + i);
      *(input + i) = *(input + len - 1 - i);
      *(input + len - 1 - i) = tmp;
  }
  *(input + len) = '\0';
  PyObject *rtnObj = Py_BuildValue("s",input);
  return rtnObj;
}

static int LinkedList_DelItem(LinkedList *self, Py_ssize_t index) {
  if (index >= self->length) {
      PyErr_SetString(PyExc_ValueError,"Out of bounds exception: Given index exceeds list bounds.");
      return -1;
  }
  LinkedListNode *node;
  if (index == 0) {
      node = self->first;
      if (self->length > 1) {
          self->first = self->first->next;
          self->first->previous = NULL;
      } else {
          self->first = NULL;
          self->last = NULL;
      }
  }
  else if (index == self->length - 1) {
      node = self->last;
      self->last = self->last->previous;
      self->last->next = NULL;
  } else {
      node = LinkedList_get_node(self, index);
      node->previous->next = node->next;
      node->next->previous = node->previous;
  }
  Py_XDECREF(node->data);
  --self->length;
  return 0;
}


static PyObject * LinkedList_fast_insert(LinkedList *self, LinkedListNode *node, int index) {
  if (!self->first) {
      node->previous = NULL;
      self->first = node;
      self->last = node;
  } else if (index == self->length) {
      node->previous = self->last;
      (self->last)->next = node;
      self->last = node;
  }
  ++self->length;
  Py_RETURN_NONE;
}

static PyObject * LinkedList_insert_impl(LinkedList *self, const int index, const PyObject *obj, const int repl) {
  if(self->pool_count == self->pool_size) {
      self->pool_size = self->pool_size + 25000;
      self->node_pool = malloc(self->pool_size  * sizeof(LinkedListNode));
      self->pool_count = 0;
      int i = 0;
      while(*(i + self->node_pool_ptrs))
        i++;
      void* tmp = realloc(self->node_pool_ptrs, (i+2) * sizeof(LinkedListNode*));
      if(!tmp) {
          PyErr_SetString(PyExc_MemoryError,"Could not reallocate memory for linked list node pool pointers.");
          return NULL;
      }
      self->node_pool_ptrs = tmp;
      *(i + self->node_pool_ptrs) = self->node_pool;
      *(i + self->node_pool_ptrs + 1) = NULL;
  }
  LinkedListNode *node = self->node_pool + self->pool_count++;
  node->data = obj;
  Py_XINCREF(obj);
  node->next = NULL;
  if (!self->first || index == self->length)
    return LinkedList_fast_insert(self,node,index);
  else {
      LinkedListNode* find_node = LinkedList_get_node(self,index);
      if(repl) {
          LinkedListNode *del_node = find_node;
          find_node = find_node->previous;
          if(!find_node)
            self->first = node;
          else
            find_node->next = node;
          node->next = del_node->next;
          if(!node->next)
            self->last = node;
          else
            (node->next)->previous = node;
          Py_XDECREF(del_node->data);
          --self->length;
      } else {
          if (find_node->previous)
            (find_node->previous)->next = node;
          node->next = find_node;
      }
  }
  ++self->length;
  Py_RETURN_NONE;
}

static PyObject * LinkedList_append(LinkedList *self, const PyObject *obj) {
  return LinkedList_insert_impl(self,self->length,obj,0);
}


static int LinkedList_SetItem(LinkedList *self, Py_ssize_t index, PyObject *obj) {
  if(index > self->length) {
      PyErr_SetString(PyExc_ValueError,"Out of bounds exception: Given index exceeds list bounds.");
      return -1;
  }
  if(obj == NULL)
    return LinkedList_DelItem(self,index);
  LinkedList_insert_impl(self,index,obj,1);
  return 0;
}

static PyObject * LinkedList_insert(LinkedList *self, PyObject *args) {
  const int index;
  const PyObject *obj;
  if(!PyArg_ParseTuple(args,"iO",&index,&obj)) {
      PyErr_SetString(PyExc_ValueError,"Linked List insert takes an index and object.");
      return NULL;
  }
  if(index > self->length) {
      PyErr_SetString(PyExc_ValueError,"Out of bounds exception: Given index exceeds list bounds.");
      return NULL;
  }
  return LinkedList_insert_impl(self,index,obj,0);
}

static LinkedListNode * LinkedList_get_node(LinkedList *self, int index) {
  LinkedListNode* find_node;
  if (index > self->length / 2) {
      find_node = self->last;
      for (int i = self->length - 1; i > index; i--)
        find_node = find_node->previous;
  } else {
      find_node = self->first;
      for (int i = 0; i < index; i++)
        find_node = find_node->next;
  }
  return find_node;
}

static const PyObject * LinkedList_GetItem(LinkedList *self, PyObject *key) {
  const int index;
  if(!PyArg_Parse(key,"i",&index))
    return NULL;
  if(index >= self->length) {
      PyErr_SetString(PyExc_ValueError,"Out of bounds exception: Given index exceeds list bounds.");
      return NULL;
  }
  LinkedListNode *node = LinkedList_get_node(self, index);
  return node->data;
}

static const PyObject * LinkedList_get(LinkedList *self, PyObject *args) {
  PyObject* index;
  if(!PyArg_ParseTuple(args,"O",&index))
    return NULL;
  return LinkedList_GetItem(self, index);
}

static Py_ssize_t LinkedList_len(LinkedList *self) {
  return self->length;
}

static PyObject * LinkedList_concat(LinkedList *a, PyObject *b) {
  if(b->ob_type != &LinkedListType) {
      PyErr_SetString(PyExc_ValueError,"Object to concatenate with must be a linked list.");
      return NULL;
  }
  LinkedList* src = (LinkedList*)b;
  LinkedListNode* node = src->first;
  if(!node)
    Py_RETURN_NONE;
  do {
      LinkedList_fast_insert(a,node,a->length);
      node = node->next;
  } while(node);
  Py_INCREF(a);
  int a_pool_ptrs = sizeof(a->node_pool_ptrs) / sizeof(LinkedListNode*);
  int src_pool_ptrs = sizeof(src->node_pool_ptrs) / sizeof(LinkedListNode*);
  void* tmp = realloc(a->node_pool_ptrs, (a_pool_ptrs + src_pool_ptrs - 1) * sizeof(LinkedListNode*));
  if(!tmp) {
      PyErr_SetString(PyExc_MemoryError,"Could not reallocate memory for linked list node pool pointers.");
      return NULL;
  }
  a->node_pool_ptrs = tmp;
  --src_pool_ptrs;
  while(src_pool_ptrs)
    *(a_pool_ptrs++ + a->node_pool_ptrs) = *(src_pool_ptrs-- + src->node_pool_ptrs);
  *(a_pool_ptrs + a->node_pool_ptrs) = NULL;
  return (PyObject *)a;
}

static PyObject * LinkedList_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  LinkedList *self;
  int ll_size;
  PyArg_ParseTuple(args,"|i",&ll_size);
  ll_size = ll_size ? ll_size : 100000;
  self = (LinkedList *)type->tp_alloc(type,0);
  if (self != NULL) {
      self->first = NULL;
      self->last = NULL;
      self->length = 0;
      self->node_pool = malloc(sizeof(LinkedListNode) * ll_size);
      self->pool_count = 0;
      self->pool_size = ll_size;
      self->node_pool_ptrs = malloc(sizeof(LinkedListNode*) * 2);
      *self->node_pool_ptrs = self->node_pool;
      *(self->node_pool_ptrs + 1) = NULL;
  }
  return (PyObject*)self;
}

static int LinkedList_init(LinkedList *self, PyObject *args, PyObject *kwds) {
  return 0;
}

static void LinkedList_dealloc(LinkedList *self) {
  while(*self->node_pool_ptrs) {
      free(*self->node_pool_ptrs);
      ++self->node_pool_ptrs;
  }
  self->first = NULL;
  self->last = NULL;
  self->length = 0;
  self->ob_type->tp_free((PyObject*)self);
}

/*********************** List Iterator **************************/

static PyObject * LinkedList_iter(LinkedList *llist)
{
  LinkedListIterator *it;

  it = PyObject_GC_New(LinkedListIterator, &LinkedListIteratorType);
  if (it == NULL)
    return NULL;
  it->it_index = 0;
  Py_XINCREF(llist);
  it->it_list = (LinkedList*) llist;
  it->it_seq = ((LinkedList*)llist)->first;
  return (PyObject *)it;
  PyErr_SetString(PyExc_ArithmeticError,"Test");
  return NULL;
}

static void
listiter_dealloc(LinkedListIterator *it)
{
  Py_XDECREF(it->it_list);
  PyObject_GC_Del(it);
}

static int
listiter_traverse(LinkedListIterator *it, visitproc visit, void *arg)
{
  Py_VISIT(it->it_seq->data);
  return 0;
}

static const PyObject *
LinkedListIter_next(LinkedListIterator *it)
{
  LinkedListNode *node;
  const PyObject *item;

  assert(it != NULL);
  node = it->it_seq;
  if (node == NULL)
    return NULL;

  if (it->it_index < it->it_list->length) {
      item = it->it_seq->data;
      ++it->it_index;
      it->it_seq = it->it_seq->next;
      return item;
  }
  it->it_seq = NULL;
  return NULL;
}

static PyObject *
listiter_len(LinkedListIterator *it)
{
  Py_ssize_t len;
  if (it->it_seq) {
      len = it->it_list->length - it->it_index;
      if (len >= 0)
        return PyInt_FromSsize_t(len);
  }
  return PyInt_FromLong(0);
}
/******************* END ITERATOR ***************************************/

static PyMethodDef InitMethods[] = {
    {"firstfnc", test_firstfnc, METH_VARARGS, "Test"},
    {"set_call_back", test_set_call_back, METH_VARARGS, "Set callback"},
    {NULL}
};

PyMODINIT_FUNC initllist(void) {
  if (PyType_Ready(&LinkedListType) < 0)
    return;
  PyObject *module = Py_InitModule("llist", InitMethods);
  if (module == NULL)
    return;
  LinkedListType.tp_as_sequence = &LinkedList_sm;
  LinkedListType.tp_as_mapping = &LinkedList_mm;
  Py_INCREF(&LinkedListType);
  PyModule_AddObject(module, "LinkedList", (PyObject *)&LinkedListType);
  TestError = PyErr_NewException("llist.error",NULL,NULL);
  Py_INCREF(TestError);
  PyModule_AddObject(module,"error",TestError);
}
