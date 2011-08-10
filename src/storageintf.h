#ifndef STORE_INTF_H
#define STORE_INTF_H

/*! @brief Abstract interface for file based memory storage operations */
class StorageIntf
{
  public:
    /*! Required by gcc */
    virtual ~StorageIntf() {}
    /*! Read \a size bytes from the store into \a buf. */
    virtual int read(char *buf,uint size) = 0;
    /*! Write \a size bytes from \a buf into the store. */
    virtual int write(const char *buf,uint size) = 0;
};

#endif // STORE_INTF_H
