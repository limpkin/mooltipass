package mooltipass.automatedTest.features;

import cucumber.api.java.en.Then;
import cucumber.api.java.en.When;
import junit.framework.Assert;
import mooltipass.automatedTest.config.WebDriverFactory;
import mooltipass.automatedTest.pageObjects.Minfin;

public class MinfinSteps {

	Minfin minfin= new Minfin(WebDriverFactory.get());

		@When("I login minfin with '(.*)' and '(.*)'")
		public void login(String username,String password){
			minfin.closePopUpIfOpen();
			minfin.goToLogin();
			minfin.enterEmail(username);
			minfin.enterPassword(password);
			minfin.submit();
			
		}
		@When("I go to minfin login page")
		public void pressLogin(){
			minfin.goToLogin();
			Assert.assertTrue("Expected to be at login page", minfin.checkAtLoginPage());
			
		}
		@Then("I should be logged in minfin")
		public void checkLogin(){
			Assert.assertTrue("Expected User to be logged in",minfin.checkLogin());
		}
		
		@When("I logout minfin")
		public void pressLogout(){
			minfin.logout();
		}
}
