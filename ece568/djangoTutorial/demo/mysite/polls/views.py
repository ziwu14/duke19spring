from django.shortcuts import render, get_object_or_404
# displays latest 5 poll questions in the system--how to use database API
from .models import Question
from django.http import HttpResponse
from django.http import Http404
###########################################################################

def index(request):
    latest_question_list = Question.objects.order_by('-pub_date')[:5]
    context = {
        'latest_question_list': latest_question_list,
    }
    # use shortcut version for render()
    # using this render(), we could remove package HttpResponse and loader by
    # directly takes html file path and the dictionary 
    return render(request,'polls/index.html',context)
'''
QuerySet API for database https://docs.djangoproject.com/en/2.1/ref/models/querysets/
to search order_by
and .join function is a Python function for delimiter
'''

def detail(request, question_id):
    question = get_object_or_404(Question, pk=question_id)
    return render(request, 'polls/detail.html', {'question': question})

def results(request, question_id):
    response = "You're looking at the results of question %s."
    return HttpResponse(response % question_id)

def vote(request, question_id):
    return HttpResponse("You're voting on question %s." % question_id)
