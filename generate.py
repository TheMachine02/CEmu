import jinja2
import os

base_dir = os.path.dirname(os.path.abspath(__file__))

templateLoader = jinja2.FileSystemLoader( searchpath=os.path.join(base_dir, "templates"))
templateEnv = jinja2.Environment(loader=templateLoader, lstrip_blocks=True, trim_blocks=True)

def generate_file(file_name):
    base_name = file_name
    file_name = file_name + ".html"
    
    # Define this if the page is located in a subdirectory instead
    # of the domain root.
    page_subdir = "/CEmu"

    template = templateEnv.get_template( file_name )
    outputText = template.render(active_page = base_name, page_subdir = page_subdir) # this is where to put args to the template renderer

    file = open(os.path.join(base_dir, "html", file_name), 'w')
    file.write(outputText)

    print("Rendered", file_name)

files = ['404', 'index', 'download']

if not os.path.isdir(os.path.join(base_dir, "html")):
    os.mkdir(os.path.join(base_dir, "html"))

for file in files:
    generate_file(file)
