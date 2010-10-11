// .NAME vtkPTXReader - read a ptx file
// .SECTION Description
// vtkPTXReader reads a ptx file

#ifndef __vtkPTXReader_h
#define __vtkPTXReader_h

#include "vtkImageAlgorithm.h"
#include "vtkSmartPointer.h"

class vtkImageData;
class vtkPolyData;

class vtkPTXReader : public vtkImageAlgorithm
{
public:
  vtkTypeMacro(vtkPTXReader,vtkImageAlgorithm);
  void PrintSelf(ostream& os, vtkIndent indent);

  static vtkPTXReader *New();

  // Description:
  // Specify file name of the ptx file.
  vtkSetStringMacro(FileName);
  vtkGetStringMacro(FileName);

  // Description:
  // Put all of the valid points into a vtkPolyData.
  void GetValidOutputPoints(vtkPolyData* output);

protected:
  vtkPTXReader();
  ~vtkPTXReader(){}

  int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *);

  char* FileName;
  vtkSmartPointer<vtkImageData> Output;

  // Description:
  // Perform the actual file input.
  void ReadFile();

private:

  vtkPTXReader(const vtkPTXReader&);  // Not implemented.
  void operator=(const vtkPTXReader&);  // Not implemented.
};

#endif
