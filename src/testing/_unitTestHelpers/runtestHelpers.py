import os
import imp
import sys
import commands

def loadModule(filename):
    if filename == '':
        raise RuntimeError, 'Empty filename cannot be loaded'
    print "Loading module %s" % (filename)
    searchPath, file = os.path.split(filename)
    if not searchPath in sys.path: 
        sys.path.append(searchPath)
        sys.path.append(os.path.normpath(searchPath+"/../"))
    moduleName, ext = os.path.splitext(file)
    fp, pathName, description = imp.find_module(moduleName, [searchPath,])

    try:
        module = imp.load_module(moduleName, fp, pathName, description)
    finally:
        if fp:
            fp.close()

    return module

def getUnitTestFiles(rootpath, testFileGlob="test_*.py"):
    rootpath = os.path.normpath(rootpath) + "/"
    print "Searching for files in %s with prefix %s" % (rootpath, testFileGlob)
    test_files = commands.getoutput("find %s -name '%s'" % (rootpath, testFileGlob))
    files = test_files.split('\n')
    if files == ['']:
        files = []
    files.sort()
    return files

