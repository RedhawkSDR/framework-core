import string

_complexTypes = ('Boolean', 'Char', 'Double', 'Float', 'Long', 'LongLong',
                 'Octet', 'Short', 'ULong', 'ULongLong', 'UShort')

_numericTypes = {
    'Double':    'double',
    'Float':     'float',
    'Long':      'int',
    'LongLong':  'long',
    'Octet':     'byte',
    'Short':     'short',
    'ULong':     'int',
    'ULongLong': 'long',
    'UShort':    'short'
}

def boxType(primitiveType):
    if primitiveType == 'int':
        return 'Integer'
    else:
        return primitiveType.title()

def generateNumericSimples():
    template = string.Template(open('NumericProperty.template').read())
    for typename, primitive in _numericTypes.items():
        mapping = {'name': typename,
                   'scaname': typename.lower(),
                   'primitive': primitive,
                   'box': boxType(primitive)}
        if typename.startswith('U'):
            mapping['parse'] = 'UnsignedUtils.parse%s' % typename
            mapping['compare'] = 'UnsignedUtils.compare%s' % typename
        elif typename in ('Double', 'Float'):
            mapping['parse'] = '%s.parse%s' % (typename, typename)
            mapping['compare'] = 'Double.compare'
        else:
            mapping['parse'] = '%s.decode' % mapping['box']
            mapping['compare'] = 'Double.compare'
        filename = typename + 'Property.java'
        open(filename, 'w').write(template.substitute(mapping))

def generateNumericSequences():
    template = string.Template(open('NumericSequenceProperty.template').read())
    for typename, primitive in _numericTypes.items():
        mapping = {'name': typename,
                   'scaname': typename.lower(),
                   'primitive': primitive.title(),
                   'box': boxType(primitive)}
        filename = typename + 'SequenceProperty.java'
        open(filename, 'w').write(template.substitute(mapping))

def generateComplexSequences():
    template = string.Template(open('ComplexSequenceProperty.template').read())
    for typename in _complexTypes:
        mapping = {'name': typename, 'scaname': typename.lower()}
        filename = 'Complex%sSequenceProperty.java' % typename
        open(filename, 'w').write(template.substitute(mapping))

def generateComplexSimples():
    template = string.Template(open('ComplexProperty.template').read())
    for typename in _complexTypes:
        mapping = {'name': typename, 'scaname': typename.lower()}
        filename = 'Complex%sProperty.java' % typename
        open(filename, 'w').write(template.substitute(mapping))

if __name__ == '__main__':
    generateNumericSimples()
    generateNumericSequences()
    generateComplexSimples()
    generateComplexSequences()
