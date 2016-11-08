pre: x == 1
{
  y = 0;
  if (x > 1) {
    x = x + 1;
  } else {
    x = x + 1;
    if (x == 100)
      y = y - 1;
    else {
      y = 99;
      x = x + 1;
      x = x - 1;
    }
  }
}
post: x == 2
