������͂ȂɁH

Direct3D �x�[�X�œ��삷��g���g�� drawdevice �v���O�C���ł��B
������ primaryLayer ���������ĕ\���ł��܂��B

���ꂼ��� primaryLayer �͎w�肳�ꂽ�̈��
�X�P�[�����O�\������邽�߁A�ʂɉ𑜓x��ύX�ł��܂��B

���g����

1. Window �� drawDevice �ɑ΂��Ďw��\�ł�

-------------------------------------------
Plugins.link("drawdeviceD3D.dll");
var WIDTH=800;
var HEIGHT=600;
class MyWindow extends Window {
  var base;
  var base2;
  function MyWindow() {
    super.Window();
    setInnerSize(WIDTH, HEIGHT);
    // drawdevice �������ւ�
    drawDevice = new DrawDeviceD3D(WIDTH,HEIGHT);
     // �v���C�}�����C������
    base = new Layer(this,null);
    base.setSize(WIDTH,HEIGHT);
    base2 = new Layer(this,null);
    base2.setSize(WIDTH/2,HEIGHT/2); // �𑜓x����
    add(base);
  }
};
-------------------------------------------

�@�\�ɂ��Ă� manual.tjs ���Q�Ƃ��Ă�������

���߂�

�Ƃ肠�����W���� PassThroughDrawDevice �̃R�[�h�����ɍ�Ƃ����̂�
Direct3D7 �x�[�X�B�C���������� D3D9 �ŏ��������H

�g���g��Z ������������ D3D9 �ɏ��������A������ DestRect �ɕs���S�Ή���
���шʒu�����m�ɔ��f����Ȃ��B
ClipRect �ɕs���S�Ή��Ńv���C�}���[���C���[���E�B���h�E�T�C�Y���傫���ꍇ�A
�͂ݏo���ĕ`�悳���B

�����C�Z���X

���̃v���O�C���̃��C�Z���X�͋g���g���{�̂ɏ������Ă��������B
