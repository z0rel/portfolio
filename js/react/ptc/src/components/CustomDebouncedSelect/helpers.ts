interface IManagerName {
  firstName: string;
  lastName: string;
}

interface IBrandName {
  title: string;
}

interface IManagerSelectFilter {
  firstName_Icontains: string | null;
  lastName_Icontains: string | null;
}

interface IClientSelectFilter {
  title_Icontains: string | undefined
}


export const getManagerName = (value: IManagerName): string => `${value.firstName} ${value.lastName}`;
export const getBrandName = (value: IBrandName): string => String(value.title);
export const managerSelectFilter = (term: string | undefined): IManagerSelectFilter => {
  const termArray = term ? term.split(' ') : [null, null];
  return { firstName_Icontains: termArray[0], lastName_Icontains: termArray[1] };
};
export const clientSelectFilter = (term: string | undefined): IClientSelectFilter => ({ title_Icontains: term });
