
class Container(object):
    def __init__(self, ref=None):
        self.ref = ref
    def getRef(self):
        return self.ref
        
class ApplicationContainer(Container):
    def __init__(self, app=None):
        super(ApplicationContainer,self).__init__(app)

class DomainManagerContainer(Container):
    def __init__(self, domMgr=None):
        super(DomainManagerContainer,self).__init__(domMgr)

class DeviceManagerContainer(Container):
    def __init__(self, devMgr=None):
        super(DeviceManagerContainer,self).__init__(devMgr)
    