// halconv.h

#ifndef HALCONV_H
#define HALCONV_H

#ifdef __cplusplus
extern "C" {
#endif

IMessagePort MessagePortNewTask( const char * name );
IMessagePort GetNamedPort();
IMessagePort CreateNamedTask( voidFunction* startRoutine, short priority, const char * name );

#ifdef __cplusplus
};
#endif

#endif	// HALCONV_H
