/**********
 randtrack_element_lock

  This version of randtrack 
  -contains a list lock when insert is called
  -no locking mechanism on the lookup
  -element locking on the count increment

  With a modification to the call signature of the insert function,
  we pass in a pointer to a pointer of sample.
  We avoid the race condition by having a list lock on the insert,
  doing another lookup within insert,
  if successful, will do a replacement on the sample ** with the 
  correct sample obtained from the lookup on the list

  hash_element_lock.h implements this modification of insert() 
  in the hash class

***********/
#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include "list.h"

#define HASH_INDEX(_addr,_size_mask) (((_addr) >> 2) & (_size_mask))

template<class Ele, class Keytype> class hash;

template<class Ele, class Keytype> class hash {
 private:
  unsigned my_size_log;
  unsigned my_size;
  unsigned my_size_mask;
  list<Ele,Keytype> *entries;
  list<Ele,Keytype> *get_list(unsigned the_idx);

  pthread_mutex_t * mutexes;

 public:
  void setup(unsigned the_size_log=5);
  void insert(Ele **e);
  Ele *lookup(Keytype the_key);
  void print(FILE *f=stdout);
  void reset();
  void cleanup();
  void lock_list(Keytype the_key);
  void unlock_list(Keytype the_key);
  
};

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::setup(unsigned the_size_log){
  my_size_log = the_size_log;
  my_size = 1 << my_size_log;
  my_size_mask = (1 << my_size_log) - 1;
  entries = new list<Ele,Keytype>[my_size];

  //initilize the array of mutexes
  
  mutexes = new pthread_mutex_t[my_size];
  for (int i=0; i< my_size; i++) {
    pthread_mutex_init(&mutexes[i], NULL);
  }
  
}

template<class Ele, class Keytype> 
list<Ele,Keytype> *
hash<Ele,Keytype>::get_list(unsigned the_idx){
  if (the_idx >= my_size){
    fprintf(stderr,"hash<Ele,Keytype>::list() public idx out of range!\n");
    exit (1);
  }
  return &entries[the_idx];
}

template<class Ele, class Keytype> 
Ele *
hash<Ele,Keytype>::lookup(Keytype the_key){
  list<Ele,Keytype> *l;

  l = &entries[HASH_INDEX(the_key,my_size_mask)];
  return l->lookup(the_key);

}  

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::print(FILE *f){
  unsigned i;

  for (i=0;i<my_size;i++){
    entries[i].print(f);
  }
}

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::reset(){
  unsigned i;
  for (i=0;i<my_size;i++){
    entries[i].cleanup();
  }
}

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::cleanup(){
  unsigned i;
  reset();
  delete [] entries;

}

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::insert(Ele **e){
  
  unsigned key = (*e)->key();
  lock_list(key);
  
  Ele * e2;

  //if not in list, we insert it normally
  if(!(e2 = lookup(key))) {
    entries[HASH_INDEX(key,my_size_mask)].push((*e));
  }
  else { //else we will replace our Ele with the one found in the list
    
    Ele * temp = (*e);
    *e = e2;
    delete temp;
    
  }
  unlock_list(key);
  
}

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::lock_list(Keytype the_key){

  pthread_mutex_lock(&mutexes[HASH_INDEX(the_key,my_size_mask)]);
 
}

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::unlock_list(Keytype the_key){

  pthread_mutex_unlock(&mutexes[HASH_INDEX(the_key,my_size_mask)]);

}

#endif
