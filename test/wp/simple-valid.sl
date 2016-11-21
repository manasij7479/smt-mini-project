pre: a==1
{
  a = a+1;
  assert: a==2
  a = a+2;
}
post: a==4
