#ifndef _SINGLETON_H_
#define _SINGLETON_H_




  template <class T>
  class Singleton
  {
  public: //! protected ctors because this is a base class
    Singleton()
    {
    }

    virtual ~Singleton()
    {
    }

  public:
    //! static method to get the unique instance of the class
    static T& instance( void )
    {
      static T the_unique_instance;
      return the_unique_instance;
    }
  };

#endif
