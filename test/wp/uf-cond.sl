pre: (a == 1) 
{
  b = [f [f a]];
  if (b > 1) {
    a = a + b;
  } else {
    a = a + 1;
  }
}
post: (a >= 1)
