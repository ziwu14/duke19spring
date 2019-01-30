# before using generic views
'''
from django.shortcuts import render, get_object_or_404
# displays latest 5 poll questions in the system--how to use database API
from .models import Question, Choice
from django.http import HttpResponse
from django.http import Http404
from django.urls import reverse
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

# QuerySet API for database https://docs.djangoproject.com/en/2.1/ref/models/querysets/
# to search order_by
# and .join function is a Python function for delimiter


def detail(request, question_id):
    question = get_object_or_404(Question, pk=question_id)
    return render(request, 'polls/detail.html', {'question': question})

# redirect to results page, only difference from detail is the template html 
def results(request, question_id):
    question = get_object_or_404(Question, pk=question_id)
    return render(request, 'polls/results.html', {'question': question})

def vote(request, question_id):
    question = get_object_or_404(Question, pk=question_id)
    try:
        selected_choice = question.choice_set.get(pk=request.POST['choice'])
    # request.POST is a dictionary-like object that lets you access submitted data by key name. In this case, request.POST['choice'] returns the ID of the selected choice, as a string. request.POST values are always strings.
    #request.POST['choice'] will raise KeyError if choice wasn’t provided in POST data. The above code checks for KeyError and redisplays the question form with an error message if choice isn’t given.
    except (KeyError, Choice.DoesNotExist):
        # Redisplay the question voting form.
        return render(request, 'polls/detail.html', {
            'question': question,
            'error_message': "You didn't select a choice.",
        })
    else:
        selected_choice.votes += 1
        selected_choice.save()
        # Always return an HttpResponseRedirect after successfully dealing
        # with POST data. This prevents data from being posted twice if a
        # user hits the Back button.
        # As the Python comment above points out, you should always return an HttpResponseRedirect after successfully dealing with POST data. This tip isn’t specific to Django; it’s just good Web development practice.
        return HttpResponseRedirect(reverse('polls:results', args=(question.id,)))
        # reverse() function--helps avoid having to hardcode a URL in the view function. It is given the name of the view that we want to pass control to and the variable portion of the URL pattern that points to that view. In this case, using the URLconf we set up in Tutorial 3, this reverse() call will return a string like
'''

# using generic views
from django.http import HttpResponseRedirect
from django.shortcuts import get_object_or_404, render
from django.urls import reverse
from django.views import generic

from .models import Choice, Question

# import generic
# define class which extends generic's template
# we could assign some attributes e.g. template, dictionary,model
#     model attribute should know which model(from models.py)
# we could define/override functions we need

# 以下用到的attributes和methods全是override

class IndexView(generic.ListView):
    template_name = 'polls/index.html'
    context_object_name = 'latest_question_list' # 纯粹是为了给dictionary换个名字

    def get_queryset(self):
        """Return the last five published questions."""
        return Question.objects.order_by('-pub_date')[:5]

# DetailView capture "pk" value from URL, that is the reason why we use "pk" instead of "question_id" in urls.py
class DetailView(generic.DetailView):
    model = Question
    template_name = 'polls/detail.html'


class ResultsView(generic.DetailView):
    model = Question
    template_name = 'polls/results.html'

def vote(request, question_id):
    question = get_object_or_404(Question, pk=question_id)
    try:
        selected_choice = question.choice_set.get(pk=request.POST['choice'])
    # request.POST is a dictionary-like object that lets you access submitted data by key name. In this case, request.POST['choice'] returns the ID of the selected choice, as a string. request.POST values are always strings.
    #request.POST['choice'] will raise KeyError if choice wasn’t provided in POST data. The above code checks for KeyError and redisplays the question form with an error message if choice isn’t given.
    except (KeyError, Choice.DoesNotExist):
        # Redisplay the question voting form.
        return render(request, 'polls/detail.html', {
            'question': question,
            'error_message': "You didn't select a choice.",
        })
    else:
        selected_choice.votes += 1
        selected_choice.save()
        # Always return an HttpResponseRedirect after successfully dealing
        # with POST data. This prevents data from being posted twice if a
        # user hits the Back button.
        # As the Python comment above points out, you should always return an HttpResponseRedirect after successfully dealing with POST data. This tip isn’t specific to Django; it’s just good Web development practice.
        return HttpResponseRedirect(reverse('polls:results', args=(question.id,)))
