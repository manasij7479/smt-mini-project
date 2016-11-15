pre: x == 1
{
  if ((x > 10) && (x < 10)) {
    x = x + 1;
  } else {
    x = x - 1;
  }
}
post: x == 2
