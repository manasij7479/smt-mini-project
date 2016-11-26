pre: z == 6
{
  def: bar {
    z = z + 1;
    z = z - 100;
  }
  def: foo {
      z = z + 1;
      assert: z==7
  }
  call: foo
  if (z>5) {
    call: foo
  } else {
  
  }
}
post: true
