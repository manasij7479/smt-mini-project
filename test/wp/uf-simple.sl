pre: [f a] == 1
{
  a = 1;
  b = a + [f 5];
}
post: b == 2
