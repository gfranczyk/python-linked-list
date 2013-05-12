#include <Python.h>
#include <structmember.h>

/****** Linked List **************/

typedef struct LinkedListNodeStruct {
  PyObject_HEAD
  struct LinkedListNodeStruct* next;
  struct LinkedListNodeStruct* previous;
  const PyObject *data;
} LinkedListNode;

typedef struct {
  PyObject_HEAD
  LinkedListNode *first;
  LinkedListNode *last;
  int length;
  LinkedListNode *node_pool;
  int pool_count;
  int pool_size;
  LinkedListNode **node_pool_ptrs;
} LinkedList;

static PyObject * LinkedList_append(LinkedList *self, const PyObject *obj);
static int LinkedList_SetItem(LinkedList *self, Py_ssize_t index, PyObject *obj);
static PyObject * LinkedList_fast_insert(LinkedList *self, LinkedListNode *node, int index);
static PyObject * LinkedList_insert(LinkedList *self, PyObject *args);
static LinkedListNode * LinkedList_get_node(LinkedList *self, int index);
static const PyObject * LinkedList_GetItem(LinkedList *self, PyObject *key);
static const PyObject * LinkedList_get(LinkedList *self, PyObject *args);
static Py_ssize_t LinkedList_len(LinkedList *self);
static PyObject * LinkedList_concat(LinkedList *a, PyObject *b);

static PyObject * LinkedList_new(PyTypeObject *type, PyObject *args, PyObject *kwds);

static int LinkedList_init(LinkedList *self, PyObject *args, PyObject *kwds);
static void LinkedList_dealloc(LinkedList *self);


/******** List Iterator **********/

typedef struct {
  PyObject_HEAD
  long it_index;
  LinkedListNode *it_seq; /* Set to NULL when iterator is exhausted */
  LinkedList *it_list; /* For general list helper methods */
} LinkedListIterator;

PyDoc_STRVAR(length_hint_doc, "Private method returning an estimate of len(list(it)).");

static PyObject *LinkedList_iter(LinkedList *);
static void listiter_dealloc(LinkedListIterator *);
static int listiter_traverse(LinkedListIterator *, visitproc, void *);
static const PyObject *LinkedListIter_next(LinkedListIterator *);
static PyObject *listiter_len(LinkedListIterator *);

/******* Type Defs **************/

static PyMemberDef LinkedList_members[] = {
    {NULL}
};

static PyMethodDef LinkedList_methods[] = {
    {"append", (PyCFunction)LinkedList_append, METH_O,
        "Add to the linked list"},
    {"get", (PyCFunction)LinkedList_get, METH_VARARGS,
        "Get object at index from linked list"},
    {"insert", (PyCFunction)LinkedList_insert, METH_VARARGS,
        "Inserts object at given index"},
    {NULL}
};

static PySequenceMethods LinkedList_sm = {
    (lenfunc)LinkedList_len, 				/* lenfunc sq_length */
    (binaryfunc)LinkedList_concat,			/* binaryfunc sq_concat */
    0,										/* ssizeargfunc sq_repeat */
    0,								 		/* ssizeargfunc sq_item */
    0,										/* ssizessizeargfunc sq_slice */
    (ssizeobjargproc)LinkedList_SetItem,	/* ssizeobjargproc sq_ass_item */
    0,										/* ssizessizeobjargproc sq_ass_slice */
    0,										/* objobjproc sq_contains */
    /* Added in release 2.0 */
    0,										/* binaryfunc sq_inplace_concat */
    0										/* ssizeargfunc sq_inplace_repeat; */
};

static PyMappingMethods LinkedList_mm = {
    (lenfunc)LinkedList_len,
    (binaryfunc)LinkedList_GetItem,
};

static PyTypeObject LinkedListType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "testc.LinkedList",             /*tp_name*/
    sizeof(LinkedList),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)LinkedList_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Linked List objects",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    (getiterfunc)LinkedList_iter,		/* tp_iter */
    0,		               /* tp_iternext */
    LinkedList_methods,             /* tp_methods */
    LinkedList_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)LinkedList_init,      /* tp_init */
    0,                         /* tp_alloc */
    LinkedList_new                 /* tp_new */

};

static PyMethodDef listiter_methods[] = {
    {"__length_hint__", (PyCFunction)listiter_len, METH_NOARGS, length_hint_doc},
    {NULL,              NULL}           /* sentinel */
};

PyTypeObject LinkedListIteratorType = {
    PyVarObject_HEAD_INIT(&PyType_Type, 0)
    "testc.LinkedListIterator",                 /* tp_name */
    sizeof(LinkedListIterator),                     /* tp_basicsize */
    0,                                          /* tp_itemsize */
    /* methods */
    (destructor)listiter_dealloc,               /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_compare */
    0,                                          /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    PyObject_GenericGetAttr,                    /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,/* tp_flags */
    0,                                          /* tp_doc */
    (traverseproc)listiter_traverse,       /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    PyObject_SelfIter,                          /* tp_iter */
    (iternextfunc)LinkedListIter_next,          /* tp_iternext */
    listiter_methods,                           /* tp_methods */
    0,                                          /* tp_members */
};
