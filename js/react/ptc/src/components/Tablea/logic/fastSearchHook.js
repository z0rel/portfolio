import { useState } from 'react';

export function useFastSearch (initialState) {
    const [fastSearchQuery, setFastSearchQuery] = useState(initialState);

    return [
        fastSearchQuery,
        newFastSearchQuery => setFastSearchQuery(newFastSearchQuery)
    ]
}
