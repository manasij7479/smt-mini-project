pre: (a == 1) && ([f a] == 1)
{
  a = a + [f a];
}
post: a==2
