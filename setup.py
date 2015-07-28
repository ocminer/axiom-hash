from distutils.core import setup, Extension

axiom_hash_module = Extension('axiom_hash',
                               sources = ['axiommodule.cpp',
                                          'sha3/shabal.c'],
                               include_dirs=['.', './sha3'])

setup (name = 'axiom_hashs',
       version = '1.0',
       description = 'Bindings for proof of work/stake used by axiom',
       ext_modules = [axiom_hash_module])    
