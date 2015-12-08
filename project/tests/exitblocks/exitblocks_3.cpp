
void test() {

}

int main() {

	int i = 0;
	while(i < 20) {
		if(i == 5) {
			test();
		}
		if(i == 10) {
			goto end_of_world;
		}
		i++;
	}

end_of_world:
	test();

	return 0;
}
