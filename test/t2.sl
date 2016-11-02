pre: x == 1
{
  y = 100;
  if ((y > 50) && (x > 1)) {
    x = x + 1;
  } else {
    x = x - 1;
  }
}
post: x == 2
