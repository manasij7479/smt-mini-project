pre: z == 6
{
  def: foo {
    if (x >= 0) {
    } else {
      y = y - 1;
      y = y + 1;
    }
    y = y + 1;
  }
  call: foo
}
post: y>5
