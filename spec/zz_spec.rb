require 'rspec'

describe 'My behaviour' do

  it 'should do something' do
    verify(0, 0)
    verify(-1, 1)
    verify(1, 2)
    verify(-2, 3)
    verify(2, 4)
    verify(2147483647, 4294967294)
    verify(2147483648, 4294967295)
  end

  def verify(n, zz)
    expect(zz_enc(n)).to eql zz
    expect(zz_dec(zz)).to eql n
  end

  def zz_enc(n)
    (n << 1) ^ (n >> 31)
  end

  def zz_dec(zz)
    # (n >> 1) ^ (n << 31)
    (zz >> 1) ^ (-(zz & 1))
  end
end