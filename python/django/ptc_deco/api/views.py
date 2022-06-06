# from django.shortcuts import render
from django.core.files.storage import FileSystemStorage
from ..logger.middleware import LoggedInUser
from django.http import JsonResponse


def file_upload(request):
    logged_in = LoggedInUser()

    if request.method == 'POST' and request.POST['id'] and request.POST['entity'] and request.FILES['file']:
        if not logged_in.current_user:
            entity = request.POST['entity'].replace('Node', '')

            file = request.FILES['construction_img']
            fs = FileSystemStorage()
            filename = fs.save(file.name, file)

            if entity == 'Construction':
                i = entity.objects.get(pk=request.POST['construction_id'])
                i.photo = filename
                i.save()

                uploaded_file_url = fs.url(filename)

                return JsonResponse({
                    'success': {
                        'url': uploaded_file_url
                    }
                })
            return JsonResponse({'error': 'badEntity'})
        return JsonResponse({'error': 'unauthorized'})
    return JsonResponse({'error': 'badRequest'})
