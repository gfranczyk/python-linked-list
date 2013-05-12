#!/usr/bin/python

from distutils.core import setup, Extension

module = Extension('llist',sources = ['llist.c'],extra_compile_args = ['-O2','-std=c99'])
setup (name = 'LinkedList', version = '0.1', description = 'Linked List Test', ext_modules= [module])
