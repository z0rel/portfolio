// Создать функцию распаковки данных по спецификации dataUnpackSpec
import { DataUnpackSpec, DataUnpackType } from './utils';

export const setDataUnpack = (dataUnpackSpec?: DataUnpackSpec, queryKey?: string): DataUnpackType | undefined => {
  if (dataUnpackSpec && queryKey) {
    if (dataUnpackSpec.unpackForLocalCompare) {
      const unpackForCompareFn = dataUnpackSpec.unpackForLocalCompare;
      return (data: any) => {
        const arr = [...data[queryKey].edges];
        arr.sort((a, b) => unpackForCompareFn(a).localeCompare(unpackForCompareFn(b)));
        return arr;
      };
    }
    else if (dataUnpackSpec.unpackNodeKey) {
      const fun = dataUnpackSpec.unpackNodeFun;
      const key = dataUnpackSpec.unpackNodeKey;
      return (data: any) => {
        const arr = [...data[queryKey].edges];
        arr.sort((a, b) => {
          const va = fun ? fun(a.node) : a.node[key];
          const vb = fun ? fun(b.node) : b.node[key];
          if (va === null || va === undefined)
            return -1;
          if (vb === null || vb === undefined)
            return 1;
          return va.localeCompare(vb);
        });
        return arr;
      };
    }
  }
  return undefined;
};
