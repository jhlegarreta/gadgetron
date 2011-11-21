#include "RemoveROOversamplingGadget.h"
#include "Gadgetron.h"
#include "FFT.h"

int RemoveROOversamplingGadget
::process(GadgetContainerMessage<GadgetMessageAcquisition>* m1,
	  GadgetContainerMessage< hoNDArray< std::complex<float> > >* m2)
{
  

  GadgetContainerMessage< hoNDArray< std::complex<float> > >* m3 
    = new GadgetContainerMessage< hoNDArray< std::complex<float> > >();

  if (!m3) {
    return GADGET_FAIL;
  }

  static int counter = 0;
  if (counter < 100) {
	  GADGET_DEBUG2("Remove oversampling Gadget, Data Added %d, %d\n",counter++,m1->getObjectPtr()->idx.line);
  }

  std::vector<unsigned int> data_out_dims = *m2->getObjectPtr()->get_dimensions();
  data_out_dims[0] = data_out_dims[0]/2;

  if (!m3->getObjectPtr()->create(&data_out_dims)) {
    GADGET_DEBUG1("Unable to create new data array for downsampled data\n");
    return GADGET_FAIL;
  }

  FFT<float>::instance()->ifft(m2->getObjectPtr(),0);
  
  std::complex<float>* data_in  = m2->getObjectPtr()->get_data_ptr();
  std::complex<float>* data_out = m3->getObjectPtr()->get_data_ptr();

  for (unsigned int c = 0; c < data_out_dims[1]; c++) {
    size_t offset_in = c*m2->getObjectPtr()->get_size(0) +  (m2->getObjectPtr()->get_size(0)-data_out_dims[0])/2;
    size_t offset_out = c*m3->getObjectPtr()->get_size(0);
    memcpy(data_out+offset_out,data_in+offset_in,data_out_dims[0]*sizeof(std::complex<float>));
  }

  FFT<float>::instance()->fft(m3->getObjectPtr(),0);
  
  m2->release(); //We are done with this data

  m1->cont(m3);
  m1->getObjectPtr()->samples = data_out_dims[0];

  if (this->next()->putq(m1) == -1) {
    ACE_ERROR_RETURN( (LM_ERROR,
		       ACE_TEXT("%p\n"),
		       ACE_TEXT("RemoveROOversamplingGadget::process, passing data on to next gadget")),
		      GADGET_FAIL);
  }

  return GADGET_OK;
}


GADGET_FACTORY_DECLARE(RemoveROOversamplingGadget)