/***************************************************************************
** CopyRight: Amlogic             
** Author   : guozheng.chen@amlogic.com
** Date     : 2018-09-11
** Description 
**  
***************************************************************************/
#ifndef _SEMAPH_H
#define _SEMAPH_H

void semOp(int, int);
int semCreate(key_t, int);
int semOpen(key_t);
void semRm(int);
void semClose(int);

#endif /* _SEMAPH_H */
