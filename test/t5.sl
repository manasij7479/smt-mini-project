pre: x == 1
{
  y = 0;
  if (x > 1) {
    x = x + 1;
  } else {
    x = x + 2;
    x = x - 1;
  }
}
post: x == 2
