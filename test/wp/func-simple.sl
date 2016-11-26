pre: a==1
{
  def: foo {
    a = a + 1;
    assert: a==2
  }
  call: foo
}
post: true
